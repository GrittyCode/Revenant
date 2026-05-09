#include "Component/RVGuardComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Data/RVWeaponDataAsset.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"

URVGuardComponent::URVGuardComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URVGuardComponent::BeginPlay()
{
    Super::BeginPlay();
}

void URVGuardComponent::InitReferences(
    ACharacter* InOwnerCharacter,
    URVCombatStateComponent* InCombatStateComponent,
    URVAttributeComponent* InAttributeComponent,
    URVEquipmentComponent* InEquipmentComponent)
{
    OwnerCharacter       = InOwnerCharacter;
    CombatStateComponent = InCombatStateComponent;
    AttributeComponent   = InAttributeComponent;
    EquipmentComponent   = InEquipmentComponent;
}

void URVGuardComponent::StartGuard()
{
    if (!CombatStateComponent->CheckAvailableState()) { return; }
    if (!CombatStateComponent->IsGrounded()) { return; }

    // AddState broadcasts OnStateChanged — SprintComponent self-terminates.
    CombatStateComponent->AddState(ERVCombatState::Guarding);
    AttributeComponent->PauseStaminaRegen();
}

void URVGuardComponent::EndGuard()
{
    if (!CombatStateComponent->HasState(ERVCombatState::Guarding)) { return; }

    CombatStateComponent->RemoveState(ERVCombatState::Guarding);
    AttributeComponent->ResumeStaminaRegen();
}

void URVGuardComponent::HandleGuardHit(float InDamageAmount)
{
    const bool bGuardHeld = AttributeComponent->ApplyStaminaDamage(InDamageAmount);
    if (!bGuardHeld) { return; }

    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData) || !IsValid(WeaponData->GetGuardHitMontage())) { return; }

    UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInstance)) { return; }

    AnimInstance->Montage_Play(WeaponData->GetGuardHitMontage());
}

void URVGuardComponent::ForceEndGuard()
{
    if (!CombatStateComponent->HasState(ERVCombatState::Guarding)) { return; }

    CombatStateComponent->RemoveState(ERVCombatState::Guarding);
    // Regen resume omitted — same reasoning as ForceEndDodge.
}

void URVGuardComponent::OnStaminaDepletedHandler()
{
    // Stamina depletion while guarding = guard break.
    // Depletion outside guard (future: sprint exhaustion, etc.) is ignored here.
    if (!CombatStateComponent->HasState(ERVCombatState::Guarding)) { return; }

    CombatStateComponent->RemoveState(ERVCombatState::Guarding);
    CombatStateComponent->AddState(ERVCombatState::GuardBroken);
    AttributeComponent->PauseStaminaRegen();

    URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData) || !IsValid(WeaponData->GetGuardBreakMontage())) { return; }

    UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInstance)) { return; }

    AnimInstance->Montage_Play(WeaponData->GetGuardBreakMontage());

    GetWorld()->GetTimerManager().SetTimer(
        GuardBreakRecoveryHandle,
        this,
        &URVGuardComponent::OnGuardBreakRecoveryComplete,
        GuardBreakRecoveryTime,
        false
    );
}

void URVGuardComponent::OnGuardBreakRecoveryComplete()
{
    CombatStateComponent->RemoveState(ERVCombatState::GuardBroken);
    AttributeComponent->ResumeStaminaRegen();
}