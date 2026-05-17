#include "Component/RVGuardComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Data/RVWeaponDataAsset.h"
#include "Data/RVPlayerCombatAnimDataAsset.h"
#include "GameFramework/Character.h"

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

    CombatStateComponent->AddState(ERVCombatState::Guarding);
}

void URVGuardComponent::EndGuard()
{
    if (!CombatStateComponent->HasState(ERVCombatState::Guarding)) { return; }

    CombatStateComponent->RemoveState(ERVCombatState::Guarding);
}

void URVGuardComponent::HandleGuardHit(float InDamageAmount)
{
    const bool bGuardHeld = AttributeComponent->ApplyStaminaDamage(InDamageAmount);
    if (!bGuardHeld) { return; }

    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData)) { return; }

    // CombatAnimData guaranteed valid by SetCurrentWeaponData ensureMsgf.
    UAnimMontage* GuardHitMontage = WeaponData->CombatAnimData->GuardHitMontage;
    if (!IsValid(GuardHitMontage)) { return; }

    UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInstance)) { return; }

    AnimInstance->Montage_Play(GuardHitMontage);
}

void URVGuardComponent::ForceEndGuard()
{
    if (!CombatStateComponent->HasState(ERVCombatState::Guarding)) { return; }

    CombatStateComponent->RemoveState(ERVCombatState::Guarding);
}

void URVGuardComponent::OnStaminaDepletedHandler()
{
    if (!CombatStateComponent->HasState(ERVCombatState::Guarding)) { return; }

    CombatStateComponent->RemoveState(ERVCombatState::Guarding);

    URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();

    UAnimMontage* GuardBreakMontage = IsValid(WeaponData)
        ? WeaponData->CombatAnimData->GuardBreakMontage : nullptr;

    OnGuardBreakTriggered.Broadcast(GuardBreakMontage);
}