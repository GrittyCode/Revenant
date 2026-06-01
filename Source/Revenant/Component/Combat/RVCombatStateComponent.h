#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVCombatStateComponent.generated.h"


enum class ERVCombatState : uint8
{
	None        = 0,
	Attacking   = 1 << 0,
	Dodging     = 1 << 1,
	Guarding    = 1 << 2,
	HitReaction = 1 << 3,
	Groggy      = 1 << 4,
	Knockdown   = 1 << 5,
};

ENUM_CLASS_FLAGS(ERVCombatState);

DECLARE_MULTICAST_DELEGATE_OneParam(FRVOnCombatStateChanged, ERVCombatState);
DECLARE_MULTICAST_DELEGATE(FRVOnForceEnd);

// Pure state authority — owns the combat state bitmask and broadcasts changes.
// Attack trace logic lives in URVAttackTraceComponent.
UCLASS(ClassGroup=(Revenant))
class REVENANT_API URVCombatStateComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVCombatStateComponent();

    //--- Delegates -----------------------------------------------------------

    FRVOnCombatStateChanged OnStateChanged;
    FRVOnForceEnd           OnForceEnd;

    //--- State Mutators ------------------------------------------------------

    void AddState(ERVCombatState InState)
    {
        CurrentStates |= InState;
        OnStateChanged.Broadcast(CurrentStates);
    }

    void RemoveState(ERVCombatState InState)
    {
        CurrentStates &= ~InState;
        OnStateChanged.Broadcast(CurrentStates);
    }

	bool HasState(ERVCombatState InState) const
    {
        return (CurrentStates & InState) != ERVCombatState::None;
    }

	void SetInvincible(bool bInInvincible) { bIsInvincible = bInInvincible; }

    //--- State Queries -------------------------------------------------------

	bool IsInvincible() const { return bIsInvincible; }

    FORCEINLINE ERVCombatState GetActiveStates() const { return CurrentStates; }

    // Returns true when no blocking state is active (optionally excluding InCoexistableStates).
    bool CheckAvailableState(ERVCombatState InCoexistableStates = ERVCombatState::None) const;

    //--- State Control -------------------------------------------------------

    void ForceEndAllActions();

private:
    ERVCombatState CurrentStates = ERVCombatState::None;
    bool           bIsInvincible = false;
};