#include "Component/Combat/RVCombatStateComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

URVCombatStateComponent::URVCombatStateComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URVCombatStateComponent::BeginPlay()
{
    Super::BeginPlay();

    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    ensureMsgf(IsValid(OwnerCharacter),
        TEXT("[URVCombatStateComponent] Owner must be ACharacter"));
    if (!IsValid(OwnerCharacter)) { return; }

    MovementComponent = OwnerCharacter->GetCharacterMovement();
    ensureMsgf(IsValid(MovementComponent),
        TEXT("[%s] URVCombatStateComponent: CharacterMovementComponent missing"),
        *GetNameSafe(OwnerCharacter));
}

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

void URVCombatStateComponent::ForceEndAllActions()
{
    OnForceEnd.Broadcast();
}
