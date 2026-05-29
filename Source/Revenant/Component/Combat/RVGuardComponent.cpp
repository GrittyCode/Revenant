#include "Component/Combat/RVGuardComponent.h"
#include "Character/Base/RVCharacterBase.h"
#include "Character/Player/RVCharacterPlayer.h"
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
    if (!ensureMsgf(IsValid(OwnerBase),
        TEXT("[URVGuardComponent] Owner must be ARVCharacterBase"))) { return; }

    ARVCharacterPlayer* OwnerPlayer = Cast<ARVCharacterPlayer>(GetOwner());
    if (!ensureMsgf(IsValid(OwnerPlayer),
        TEXT("[URVGuardComponent] Player-only component — owner must be ARVCharacterPlayer"))) { return; }

    StaminaComponent   = OwnerPlayer->GetStaminaComponent();
    EquipmentComponent = OwnerPlayer->GetEquipmentComponent();
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
    if (!ensureMsgf(IsValid(WeaponData),
        TEXT("[%s] HandleGuardHit: WeaponData not assigned"), *GetNameSafe(OwnerBase))) { return; }
    if (!ensureMsgf(IsValid(WeaponData->CombatAnimData),
        TEXT("[%s] HandleGuardHit: CombatAnimData not assigned"), *GetNameSafe(OwnerBase))) { return; }

    UAnimMontage* GuardHitMontage = WeaponData->CombatAnimData->GuardHitMontage;
    if (!ensureMsgf(IsValid(GuardHitMontage),
        TEXT("[%s] HandleGuardHit: GuardHitMontage not assigned"), *GetNameSafe(OwnerBase))) { return; }

    UAnimInstance* AnimInst = OwnerBase->GetMesh()->GetAnimInstance();
    if (!ensureMsgf(IsValid(AnimInst),
        TEXT("[%s] HandleGuardHit: AnimInstance missing"), *GetNameSafe(OwnerBase))) { return; }

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