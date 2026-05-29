#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVCombatStateComponent.generated.h"

class UCharacterMovementComponent;

enum class ERVCombatState : uint8
{
	None           = 0,
	Attacking      = 1 << 0,
	HeavyAttacking = 1 << 1,
	Dodging        = 1 << 2,
	Guarding       = 1 << 3,
	HeavyCharging  = 1 << 4,
	HitReaction    = 1 << 5,
	Groggy         = 1 << 6,
	Knockdown      = 1 << 7,
};

ENUM_CLASS_FLAGS(ERVCombatState);

DECLARE_MULTICAST_DELEGATE_OneParam(FRVOnCombatStateChanged, ERVCombatState);
DECLARE_MULTICAST_DELEGATE(FRVOnForceEnd);

// Pure state authority — owns the combat state bitmask and broadcasts changes.
// Attack trace logic lives in URVAttackTraceComponent.
UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVCombatStateComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVCombatStateComponent();

    //--- Delegates -----------------------------------------------------------

    FRVOnCombatStateChanged OnStateChanged;
    FRVOnForceEnd           OnForceEnd;

    //--- State Mutators ------------------------------------------------------

    FORCEINLINE void AddState(ERVCombatState InState)
    {
        CurrentStates |= InState;
        OnStateChanged.Broadcast(CurrentStates);
    }

    FORCEINLINE void RemoveState(ERVCombatState InState)
    {
        CurrentStates &= ~InState;
        OnStateChanged.Broadcast(CurrentStates);
    }

    FORCEINLINE bool HasState(ERVCombatState InState) const
    {
        return (CurrentStates & InState) != ERVCombatState::None;
    }

    void SetInvincible(bool bInInvincible) { bIsInvincible = bInInvincible; }

    //--- State Queries -------------------------------------------------------

    bool IsInState(ERVCombatState InState) const { return HasState(InState); }
    bool IsInvincible() const { return bIsInvincible; }
    bool IsGrounded() const;

    ERVCombatState GetActiveStates() const { return CurrentStates; }

    // Returns true when no blocking state is active (optionally excluding InCoexistableStates).
    bool CheckAvailableState(ERVCombatState InCoexistableStates = ERVCombatState::None) const;

    //--- State Control -------------------------------------------------------

    // Broadcasts OnForceEnd — subscribers (WeaponAttackComponent, GuardComponent, etc.)
    // each terminate their own action. Wired by the owning Actor in BeginPlay.
    void ForceEndAllActions();

protected:
    virtual void BeginPlay() override;

private:
    // Resolved in BeginPlay via Cast<ACharacter>(GetOwner())->GetCharacterMovement().
    UPROPERTY()
    TObjectPtr<UCharacterMovementComponent> MovementComponent;

    ERVCombatState CurrentStates = ERVCombatState::None;
    bool           bIsInvincible = false;
};
