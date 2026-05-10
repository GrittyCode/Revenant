#include "Component/RVCombatStateComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Data/RVWeaponDataAsset.h"
#include "Interface/RVDamageable.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"

URVCombatStateComponent::URVCombatStateComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URVCombatStateComponent::BeginPlay()
{
    Super::BeginPlay();
    // References injected via InitReferences() from ARVCharacterBase::BeginPlay.
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
    // Broadcast so subscribers (e.g. SprintComponent) can react without being
    // called directly by action components.
    OnStateChanged.Broadcast(CurrentStates);
}

//--- State Queries -----------------------------------------------------------

bool URVCombatStateComponent::IsGrounded() const
{
    return IsValid(MovementComponent) && !MovementComponent->IsFalling();
}

bool URVCombatStateComponent::CheckAvailableState(ERVCombatState InCoexistableStates) const
{
    // All states that block new action input.
    // HitReaction / Groggy / Knockdown added in Phase 3 — a character taking damage
    // cannot start new actions while staggered, groggy, or knocked down.
    const ERVCombatState BlockingStates =
        ERVCombatState::Attacking      |
        ERVCombatState::HeavyCharging  |
        ERVCombatState::HeavyAttacking |
        ERVCombatState::Dodging        |
        ERVCombatState::Guarding       |
        ERVCombatState::GuardBroken    |
        ERVCombatState::HitReaction    |
        ERVCombatState::Groggy         |
        ERVCombatState::Knockdown;

    const ERVCombatState Relevant = (CurrentStates & BlockingStates) & ~InCoexistableStates;
    return Relevant == ERVCombatState::None;
}

//--- State Control -----------------------------------------------------------

void URVCombatStateComponent::ForceEndAllActions()
{
    // Each subscribed action component cleans up its own state and timers.
    // HitReaction / Groggy / Knockdown are NOT cleared here —
    // URVHitReactionComponent owns their lifetime.
    OnForceEnd.Broadcast();
}

void URVCombatStateComponent::OnAttackStarted()
{
    AddState(ERVCombatState::Attacking);
    // Guard → attack transition: bypass EndGuard to keep stamina regen paused.
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

float URVCombatStateComponent::ResolveDamage(const URVWeaponDataAsset* InWeaponData) const
{
    if (HasState(ERVCombatState::HeavyAttacking))
    {
        return (ActiveTier == ERVHeavyAttackTier::AutoRelease)
            ? InWeaponData->HeavyAttackDamage_Max
            : InWeaponData->HeavyAttackDamage;
    }
    return InWeaponData->AttackDamage;
}

void URVCombatStateComponent::PerformAttackTrace()
{
    URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData)) { return; }

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
        FCollisionShape::MakeCapsule(WeaponData->AttackRadius, HalfHeight),
        Params
    );

#if !UE_BUILD_SHIPPING
    DrawDebugCapsule(GetWorld(), Center, HalfHeight, WeaponData->AttackRadius,
                     Rotation, FColor::Red, false, 1.f);
#endif

    // Determine if this hit should force knockdown.
    // Heavy auto-release with bHeavyAttackForceKnockdown set = guaranteed knockdown.
    const bool bForceKnockdown = WeaponData->bHeavyAttackForceKnockdown
                               && HasState(ERVCombatState::HeavyAttacking)
                               && ActiveTier == ERVHeavyAttackTier::AutoRelease;

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
            HitInfo.Damage          = ResolveDamage(WeaponData);
            HitInfo.PoiseDamage     = WeaponData->PoiseDamage;
            HitInfo.bForceKnockdown = bForceKnockdown;
            // Direction from attacker toward target — used by HitReactionComponent
            // to select directional stagger montage and drive ABP additive layer.
            HitInfo.HitDirection    = (HitActor->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal();
            HitInfo.Instigator      = OwnerCharacter;

            Target->ApplyDamage(HitInfo);
        }
    }
}