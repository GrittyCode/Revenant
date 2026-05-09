#include "Component/RVDodgeComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Data/RVWeaponDataAsset.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"

URVDodgeComponent::URVDodgeComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URVDodgeComponent::BeginPlay()
{
    Super::BeginPlay();
}

void URVDodgeComponent::InitReferences(
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

void URVDodgeComponent::StartDodge(const FVector& InDodgeDirection)
{
    if (!CombatStateComponent->CheckAvailableState(ERVCombatState::Guarding)) { return; }
    if (!CombatStateComponent->IsGrounded()) { return; }

    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData) || !IsValid(WeaponData->GetDodgeMontage())) { return; }

    if (!AttributeComponent->ConsumeStamina(WeaponData->DodgeStaminaCost)) { return; }

    // Guard state cleanup — no EndGuard call to keep regen paused.
    if (CombatStateComponent->HasState(ERVCombatState::Guarding))
    {
        CombatStateComponent->RemoveState(ERVCombatState::Guarding);
    }

    UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInstance)) { return; }

    OwnerCharacter->SetActorRotation(InDodgeDirection.ToOrientationRotator());
    OwnerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;

    // AddState broadcasts OnStateChanged — SprintComponent self-terminates.
    CombatStateComponent->AddState(ERVCombatState::Dodging);
    AttributeComponent->PauseStaminaRegen();

    FOnMontageBlendingOutStarted BlendOutDelegate;
    BlendOutDelegate.BindUObject(this, &URVDodgeComponent::OnDodgeMontageBlendingOut);

    UAnimMontage* DodgeMontage = WeaponData->GetDodgeMontage();
    AnimInstance->Montage_Play(DodgeMontage);
    AnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, DodgeMontage);
}

void URVDodgeComponent::SetDodgeIFrame(bool bActivate)
{
    // Prevent i-frame activation if dodge was interrupted before the window opened.
    if (!CombatStateComponent->HasState(ERVCombatState::Dodging) && bActivate) { return; }
    CombatStateComponent->SetInvincible(bActivate);
}

void URVDodgeComponent::ForceEndDodge()
{
    if (!CombatStateComponent->HasState(ERVCombatState::Dodging)) { return; }

    CombatStateComponent->RemoveState(ERVCombatState::Dodging);
    CombatStateComponent->SetInvincible(false);
    OwnerCharacter->GetCharacterMovement()->bOrientRotationToMovement = true;
    // Regen resume omitted: ForceEndAllActions is called on hit —
    // regen will be managed by the hit reaction system (Phase 3).
}

void URVDodgeComponent::EndDodge()
{
    if (!CombatStateComponent->HasState(ERVCombatState::Dodging)) { return; }

    CombatStateComponent->RemoveState(ERVCombatState::Dodging);
    CombatStateComponent->SetInvincible(false);
    OwnerCharacter->GetCharacterMovement()->bOrientRotationToMovement = true;
    AttributeComponent->ResumeStaminaRegen();
}

void URVDodgeComponent::OnDodgeMontageBlendingOut(UAnimMontage* /*InMontage*/, bool /*bInterrupted*/)
{
    EndDodge();
}