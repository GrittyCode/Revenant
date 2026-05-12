// Source/Revenant/Component/RVCombatStateComponent.cpp
#include "Component/RVCombatStateComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Data/RVWeaponDataAsset.h"
#include "Data/RVMontageStatData.h"
#include "Data/RVAttackActionMultiplierRow.h"
#include "Data/RVWeaponStatRow.h"
#include "Interface/RVDamageable.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
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
    URVEquipmentComponent* InEquipmentComponent,
    UCharacterMovementComponent* InMovementComponent)
{
    OwnerCharacter     = InOwnerCharacter;
    EquipmentComponent = InEquipmentComponent;
    MovementComponent  = InMovementComponent;
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
    URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData)) { return; }

    // --- Resolve stats ------------------------------------------------------
    // Base values come from the weapon (DT_WeaponStats).
    // Multipliers come from the currently playing montage's UserData (DT_AttackStats).
    // Final = base × multiplier — weapons sharing the same animation style
    // can still have different damage output.

    const FRVWeaponStatRow* WeaponStat = WeaponData->GetWeaponStatRow();

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

    const float Damage      = BaseDamage      * DmgMult;
    const float PoiseDamage = BasePoiseDamage * PoiseMult;
    const ERVHitType HitType = AttackStat ? AttackStat->HitType : ERVHitType::Normal;
    const float AttackRadius = 40.0f; // Hard Coding  -> Weapon Socket

    // --- Capsule sweep -------------------------------------------------------

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    const FVector Root = Mesh->GetSocketLocation(FName("WeaponRoot"));
    const FVector Tip  = Mesh->GetSocketLocation(FName("WeaponTip"));

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

    // --- Damage application --------------------------------------------------

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
            HitInfo.HitDirection = (HitActor->GetActorLocation()
                                  - OwnerCharacter->GetActorLocation()).GetSafeNormal();
            HitInfo.Instigator   = OwnerCharacter;

            Target->ApplyDamage(HitInfo);
        }
    }
}