#include "Component/Combat/RVGuardComponent.h"
#include "Character/Base/RVCharacterBase.h"
#include "Component/Attribute/RVStaminaComponent.h"
#include "Component/Utility/RVEquipmentComponent.h"
#include "Data/Asset/RVWeaponDataAsset.h"
#include "Data/Asset/RVPlayerCombatAnimDataAsset.h"
#include "Animation/AnimInstance.h"

URVGuardComponent::URVGuardComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URVGuardComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerBase = Cast<ARVCharacterBase>(GetOwner());
    ensureMsgf(IsValid(OwnerBase),
        TEXT("[URVGuardComponent] Owner must be ARVCharacterBase"));
}

void URVGuardComponent::Init(
    URVStaminaComponent* InStamina, URVEquipmentComponent* InEquipment)
{
    ensureMsgf(IsValid(InStamina),   TEXT("[URVGuardComponent] Init: StaminaComponent is null"));
    ensureMsgf(IsValid(InEquipment), TEXT("[URVGuardComponent] Init: EquipmentComponent is null"));
    StaminaComponent   = InStamina;
    EquipmentComponent = InEquipment;
}

void URVGuardComponent::StartGuard()
{
    if (!OwnerBase->CanAct())     { return; }
    if (!OwnerBase->IsGrounded()) { return; }
    OwnerBase->AddCombatState(ERVCombatState::Guarding);
}

void URVGuardComponent::EndGuard()
{
    if (!OwnerBase->HasCombatState(ERVCombatState::Guarding)) { return; }
    OwnerBase->RemoveCombatState(ERVCombatState::Guarding);
}

void URVGuardComponent::HandleGuardHit(float InDamageAmount)
{
    const bool bGuardHeld = StaminaComponent->ApplyStaminaDamage(InDamageAmount);
    if (!bGuardHeld) { return; }

    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData) || !IsValid(WeaponData->CombatAnimData)) { return; }

    UAnimMontage* GuardHitMontage = WeaponData->CombatAnimData->GuardHitMontage;
    if (!IsValid(GuardHitMontage)) { return; }

    UAnimInstance* AnimInst = OwnerBase->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    AnimInst->Montage_Play(GuardHitMontage);
}

void URVGuardComponent::OnStaminaDepletedHandler()
{
    if (!OwnerBase->HasCombatState(ERVCombatState::Guarding)) { return; }

    OwnerBase->RemoveCombatState(ERVCombatState::Guarding);

    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    UAnimMontage* GuardBreakMontage = (IsValid(WeaponData) && IsValid(WeaponData->CombatAnimData))
        ? WeaponData->CombatAnimData->GuardBreakMontage : nullptr;

    OwnerBase->TriggerStaggerWithMontage(GuardBreakMontage);
}