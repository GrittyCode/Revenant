#include "Component/RVGuardComponent.h"
#include "Character/Base/RVCharacterBase.h"
#include "Interface/RVWeaponUser.h"
#include "Data/RVWeaponDataAsset.h"
#include "Data/RVPlayerCombatAnimDataAsset.h"
#include "Animation/AnimInstance.h"

URVGuardComponent::URVGuardComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URVGuardComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerBase  = Cast<ARVCharacterBase>(GetOwner());
    WeaponUser = Cast<IRVWeaponUser>(GetOwner());

    ensureMsgf(IsValid(OwnerBase),
        TEXT("[URVGuardComponent] Owner must be ARVCharacterBase"));
    ensureMsgf(WeaponUser != nullptr,
        TEXT("[URVGuardComponent] Owner must implement IRVWeaponUser (e.g. ARVCharacterPlayer)"));

    // OnStaminaDepleted and OnForceEnd subscriptions are wired by ARVCharacterPlayer::BeginPlay.
    // Components do not self-subscribe to sibling component delegates.
}

void URVGuardComponent::StartGuard()
{
    if (!OwnerBase->CanAct())    { return; }
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
    const bool bGuardHeld = OwnerBase->ApplyStaminaDamage(InDamageAmount);
    if (!bGuardHeld) { return; }

    const URVWeaponDataAsset* WeaponData = WeaponUser->GetCurrentWeaponData();
    if (!ensureMsgf(IsValid(WeaponData),
        TEXT("[%s] HandleGuardHit: WeaponData not assigned"), *GetNameSafe(OwnerBase))) { return; }

    UAnimMontage* GuardHitMontage = WeaponData->CombatAnimData->GuardHitMontage;
    if (!ensureMsgf(IsValid(GuardHitMontage),
        TEXT("[%s] HandleGuardHit: GuardHitMontage not assigned"), *GetNameSafe(OwnerBase))) { return; }

    UAnimInstance* AnimInst = OwnerBase->GetMesh()->GetAnimInstance();
    ensureMsgf(IsValid(AnimInst),
        TEXT("[%s] HandleGuardHit: AnimInstance missing"), *GetNameSafe(OwnerBase));
    if (!IsValid(AnimInst)) { return; }

    AnimInst->Montage_Play(GuardHitMontage);
}

void URVGuardComponent::OnStaminaDepletedHandler()
{
    if (!OwnerBase->HasCombatState(ERVCombatState::Guarding)) { return; }

    OwnerBase->RemoveCombatState(ERVCombatState::Guarding);

    const URVWeaponDataAsset* WeaponData = WeaponUser->GetCurrentWeaponData();
    UAnimMontage* GuardBreakMontage = IsValid(WeaponData)
        ? WeaponData->CombatAnimData->GuardBreakMontage : nullptr;

    // Route through CharacterBase interface — GuardComponent does not hold
    // a reference to HitReactionComponent.
    OwnerBase->TriggerStaggerWithMontage(GuardBreakMontage);
}
