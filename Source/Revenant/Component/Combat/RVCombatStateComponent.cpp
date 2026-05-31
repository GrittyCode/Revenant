#include "Component/Combat/RVCombatStateComponent.h"

URVCombatStateComponent::URVCombatStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool URVCombatStateComponent::CheckAvailableState(ERVCombatState InCoexistableStates) const
{
	const ERVCombatState BlockingStates =
		ERVCombatState::Attacking   |
		ERVCombatState::Dodging     |
		ERVCombatState::Guarding    |
		ERVCombatState::HitReaction |
		ERVCombatState::Groggy      |
		ERVCombatState::Knockdown;

	const ERVCombatState Relevant = (CurrentStates & BlockingStates) & ~InCoexistableStates;
	return Relevant == ERVCombatState::None;
}

void URVCombatStateComponent::ForceEndAllActions()
{
	OnForceEnd.Broadcast();
}