#include "Component/RVCombatStateComponent.h"
#include "Data/RVCombatDataAsset.h"
#include "Data/RVMontageStatData.h"
#include "Data/RVAttackActionMultiplierRow.h"
#include "Data/RVWeaponStatRow.h"
#include "Interface/RVDamageable.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Components/MeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"

URVCombatStateComponent::URVCombatStateComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URVCombatStateComponent::BeginPlay()
{
    Super::BeginPlay();
}

void URVCombatStateComponent::InitReferences(
    ACharacter* InOwnerCharacter,
    URVCombatDataAsset* InCombatData,
    UMeshComponent* InTraceMesh,
    UCharacterMovementComponent* InMovementComponent)
{
    OwnerCharacter    = InOwnerCharacter;
    CombatData        = InCombatData;
    TraceMesh         = InTraceMesh;
    MovementComponent = InMovementComponent;
}

//--- State Mutators ----------------------------------------------------------

void URVCombatStateComponent::AddState(ERVCombatState InState)
{
    CurrentStates |= InState;
    OnStateChanged.Broadcast(CurrentStates);
}

//--- State Queries -----------------------------------------------------------

bool URVCombatStateComponent::IsGrounded() const
{
    return IsValid(MovementComponent) && !MovementComponent->IsFalling();
}

bool URVCombatStateComponent::CheckAvailableState(ERVCombatState InCoexistableStates) const
{
    const ERVCombatState BlockingStates =
        ERVCombatState::Attacking      |
        ERVCombatState::HeavyCharging  |
        ERVCombatState::HeavyAttacking |
        ERVCombatState::Dodging        |
        ERVCombatState::Guarding       |
        ERVCombatState::HitReaction    |
        ERVCombatState::Groggy         |
        ERVCombatState::Knockdown;

    const ERVCombatState Relevant = (CurrentStates & BlockingStates) & ~InCoexistableStates;
    return Relevant == ERVCombatState::None;
}

//--- State Control -----------------------------------------------------------

void URVCombatStateComponent::ForceEndAllActions()
{
    OnForceEnd.Broadcast();
}

void URVCombatStateComponent::OnAttackStarted()
{
    AddState(ERVCombatState::Attacking);
    if (HasState(ERVCombatState::Guarding)) { RemoveState(ERVCombatState::Guarding); }
}

void URVCombatStateComponent::OnAttackEnded()
{
    RemoveState(ERVCombatState::Attacking);
}

//--- Attack Trace ------------------------------------------------------------

void URVCombatStateComponent::OpenHitWindow()
{
    HitActors.Empty();
}

void URVCombatStateComponent::CloseHitWindow()
{
    HitActors.Empty();
}

void URVCombatStateComponent::PerformAttackTrace()
{
    if (!IsValid(CombatData)) { return; }

    const FRVWeaponStatRow* WeaponStat = CombatData->GetWeaponStatRow();

    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    UAnimMontage* CurrentMontage = AnimInst->GetCurrentActiveMontage();
    const URVMontageStatData* StatData = CurrentMontage
        ? CurrentMontage->GetAssetUserData<URVMontageStatData>()
        : nullptr;

    const FRVAttackActionMultiplierRow* AttackStat = StatData ? StatData->GetStatRow() : nullptr;

    const float BaseDamage      = WeaponStat ? WeaponStat->BaseDamage      : 0.f;
    const float BasePoiseDamage = WeaponStat ? WeaponStat->BasePoiseDamage  : 0.f;
    const float DmgMult         = AttackStat ? AttackStat->DamageMultiplier      : 1.f;
    const float PoiseMult       = AttackStat ? AttackStat->PoiseDamageMultiplier : 1.f;
    const float AttackRadius    = WeaponStat ? WeaponStat->AttackRadius     : 40.f;

    const float Damage      = BaseDamage      * DmgMult;
    const float PoiseDamage = BasePoiseDamage * PoiseMult;
    const ERVHitType HitType = AttackStat ? AttackStat->HitType : ERVHitType::Normal;

    // --- Socket source -------------------------------------------------------
    // Player: weapon StaticMeshComponent (WeaponRoot/WeaponTip on weapon mesh)
    // Boss:   character SkeletalMeshComponent (WeaponRoot/WeaponTip on Sevarog skeleton)

    if (!IsValid(TraceMesh))
    {
        ensureMsgf(false,
            TEXT("[%s] PerformAttackTrace: TraceMesh is null — assign via GetWeaponTraceMesh()"),
            *GetNameSafe(OwnerCharacter));
        return;
    }

    if (!TraceMesh->DoesSocketExist(FName("WeaponRoot")) ||
        !TraceMesh->DoesSocketExist(FName("WeaponTip")))
    {
        ensureMsgf(false,
            TEXT("[%s] PerformAttackTrace: WeaponRoot or WeaponTip socket missing"),
            *GetNameSafe(OwnerCharacter));
        return;
    }

    const FVector Root = TraceMesh->GetSocketLocation(FName("WeaponRoot"));
    const FVector Tip  = TraceMesh->GetSocketLocation(FName("WeaponTip"));

    const FVector Center     = (Root + Tip) * 0.5f;
    const float   HalfHeight = FVector::Dist(Root, Tip) * 0.5f;
    const FQuat   Rotation   = FRotationMatrix::MakeFromZ(Tip - Root).ToQuat();

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    TArray<FOverlapResult> Overlaps;
    // TODO: replace ECC_Pawn with project-specific channel once RVCollision.h is defined
    GetWorld()->OverlapMultiByChannel(
        Overlaps, Center, Rotation, ECC_Pawn,
        FCollisionShape::MakeCapsule(AttackRadius, HalfHeight),
        Params
    );

#if !UE_BUILD_SHIPPING
    DrawDebugCapsule(GetWorld(), Center, HalfHeight, AttackRadius,
                     Rotation, FColor::Red, false, 1.f);
#endif

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* HitActor = Overlap.GetActor();
        if (!IsValid(HitActor)) { continue; }

        TWeakObjectPtr<AActor> WeakHitActor(HitActor);
        if (HitActors.Contains(WeakHitActor)) { continue; }
        HitActors.Add(WeakHitActor);

        if (IRVDamageable* Target = Cast<IRVDamageable>(HitActor))
        {
            FRVHitInfo HitInfo;
            HitInfo.Damage       = Damage;
            HitInfo.PoiseDamage  = PoiseDamage;
            HitInfo.HitType      = HitType;
            HitInfo.Instigator   = OwnerCharacter;
            HitInfo.HitDirection = (HitActor->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal();

            Target->ApplyDamage(HitInfo);
        }
    }
}