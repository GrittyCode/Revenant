#include "Component/RVDodgeComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVAttributeComponent.h"
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
    float InDodgeStaminaCost)
{
    OwnerCharacter     = InOwnerCharacter;
    CombatStateComponent = InCombatStateComponent;
    AttributeComponent   = InAttributeComponent;
    DodgeStaminaCost     = InDodgeStaminaCost;
}

bool URVDodgeComponent::CanStartDodge() const
{
    if (CombatStateComponent->HasState(ERVCombatState::Dodging)) { return false; }
    if (!CombatStateComponent->CheckAvailableState())            { return false; }
    if (!CombatStateComponent->IsGrounded())                     { return false; }
    if (AttributeComponent->GetCurrentStamina() < DodgeStaminaCost) { return false; }

    return true;
}

void URVDodgeComponent::StartDodge(UAnimMontage* InMontage)
{
    if (CombatStateComponent->HasState(ERVCombatState::Dodging)) { return; }
    if (!CombatStateComponent->CheckAvailableState())            { return; }
    if (!CombatStateComponent->IsGrounded())                     { return; }
    if (!IsValid(InMontage))                                     { return; }

    if (!AttributeComponent->ConsumeStamina(DodgeStaminaCost)) { return; }

    UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInstance)) { return; }

    OwnerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
    AttributeComponent->PauseStaminaRegen();
    CombatStateComponent->AddState(ERVCombatState::Dodging);

    ActiveDodgeMontage = InMontage;

    FOnMontageBlendingOutStarted BlendOutDelegate;
    BlendOutDelegate.BindUObject(this, &URVDodgeComponent::OnDodgeMontageBlendingOut);

    AnimInstance->Montage_Play(InMontage);
    AnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, InMontage);
}

void URVDodgeComponent::EndDodge()
{
    if (!CombatStateComponent->HasState(ERVCombatState::Dodging)) { return; }

    ActiveDodgeMontage = nullptr;

    CombatStateComponent->RemoveState(ERVCombatState::Dodging);
    CombatStateComponent->SetInvincible(false);
    OwnerCharacter->GetCharacterMovement()->bOrientRotationToMovement = true;
    AttributeComponent->ResumeStaminaRegen();
}

void URVDodgeComponent::SetDodgeIFrame(bool bActivate)
{
    if (!CombatStateComponent->HasState(ERVCombatState::Dodging) && bActivate) { return; }
    CombatStateComponent->SetInvincible(bActivate);
}

void URVDodgeComponent::ForceEndDodge()
{
    EndDodge();
}

void URVDodgeComponent::OnDodgeMontageBlendingOut(UAnimMontage* InMontage, bool /*bInterrupted*/)
{
    if (InMontage != ActiveDodgeMontage) { return; }
    EndDodge();
}