#include "Component/RVCombatStateComponent.h"
#include "Data/RVMontageStatData.h"
#include "Data/RVAttackActionMultiplierRow.h"
#include "Interface/RVDamageable.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Components/MeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Particles/ParticleSystem.h"

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
    UMeshComponent* InTraceMesh,
    UCharacterMovementComponent* InMovementComponent)
{
    OwnerCharacter    = InOwnerCharacter;
    TraceMesh         = InTraceMesh;
    MovementComponent = InMovementComponent;
}

void URVCombatStateComponent::SetCombatStat(
    float InBaseDamage,
    float InBasePoiseDamage,
    float InAttackRadius)
{
    CachedBaseDamage      = InBaseDamage;
    CachedBasePoiseDamage = InBasePoiseDamage;
    CachedAttackRadius    = InAttackRadius;
}

void URVCombatStateComponent::SetHitFX(
    UNiagaraSystem* InNiagara,
    UParticleSystem* InCascade,
    USoundBase* InSFX)
{
    HitImpactEffect        = InNiagara;
    HitImpactEffectCascade = InCascade;
    HitSFX                 = InSFX;
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
    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    UAnimMontage* CurrentMontage = AnimInst->GetCurrentActiveMontage();
    const URVMontageStatData* StatData = CurrentMontage
        ? CurrentMontage->GetAssetUserData<URVMontageStatData>()
        : nullptr;

    const FRVAttackActionMultiplierRow* AttackStat = StatData ? StatData->GetStatRow() : nullptr;

    const float DmgMult   = AttackStat ? AttackStat->DamageMultiplier      : 1.f;
    const float PoiseMult = AttackStat ? AttackStat->PoiseDamageMultiplier : 1.f;
    const float Damage      = CachedBaseDamage      * DmgMult;
    const float PoiseDamage = CachedBasePoiseDamage * PoiseMult;

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
        FCollisionShape::MakeCapsule(CachedAttackRadius, HalfHeight),
        Params
    );

#if !UE_BUILD_SHIPPING
    DrawDebugCapsule(GetWorld(), Center, HalfHeight, CachedAttackRadius,
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
            HitInfo.Instigator   = OwnerCharacter;
            HitInfo.HitDirection = (OwnerCharacter->GetActorLocation() - HitActor->GetActorLocation()).GetSafeNormal();

            Target->ApplyDamage(HitInfo);

            // ---- Hit Impact VFX ----
            const FVector ImpactLocation = HitActor->GetActorLocation();

            if (IsValid(HitImpactEffect))
            {
                UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                    GetWorld(), HitImpactEffect, ImpactLocation);
            }
            else if (IsValid(HitImpactEffectCascade))
            {
                UGameplayStatics::SpawnEmitterAtLocation(
                    GetWorld(), HitImpactEffectCascade, ImpactLocation);
            }

            // ---- Hit SFX ----
            if (IsValid(HitSFX))
            {
                UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSFX, ImpactLocation);
            }
        }
    }
}