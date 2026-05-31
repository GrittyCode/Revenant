#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVCombatStateComponent.generated.h"

// [수정] HeavyCharging / HeavyAttacking 제거.
//         두 값은 URVWeaponAttackComponent 내부 페이즈 구분에만 쓰였으며,
//         외부 시스템은 항상 셋을 OR로 묶어 Attacking과 동일하게 취급했다.
//         내부 페이즈 추적은 WeaponAttackComponent의 EHeavyPhase 멤버로 이전.
//         8비트 → 6비트로 단순화. 런타임 전용 상태이므로 재번호 부여 가능.
UENUM(BlueprintType, meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
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

    bool IsInvincible() const { return bIsInvincible; }

    ERVCombatState GetActiveStates() const { return CurrentStates; }

    // Returns true when no blocking state is active (optionally excluding InCoexistableStates).
    bool CheckAvailableState(ERVCombatState InCoexistableStates = ERVCombatState::None) const;

    //--- State Control -------------------------------------------------------

    void ForceEndAllActions();

private:
    ERVCombatState CurrentStates = ERVCombatState::None;
    bool           bIsInvincible = false;
};