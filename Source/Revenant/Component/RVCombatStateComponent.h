#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVCombatStateComponent.generated.h"

class ACharacter;
class URVEquipmentComponent;
class UCharacterMovementComponent;
class UAnimMontage;
class URVWeaponDataAsset;

/**
 * Combat action state bitmask.
 * Bit allocation:
 *   0-5  : action states (Attacking … HeavyCharging)
 *   6-8  : hit-reaction states (HitReaction, Groggy, Knockdown)   ← Phase 3
 *   9-10 : execution states (Executing, BeingExecuted)             ← Phase 5 reserved
 */
UENUM(meta=(Bitflags))
enum class ERVCombatState : uint16
{
    None           = 0,
    Attacking      = 1 << 0,
    HeavyAttacking = 1 << 1,
    Dodging        = 1 << 2,
    Guarding       = 1 << 3,
    GuardBroken    = 1 << 4,
    HeavyCharging  = 1 << 5,
	HitReaction    = 1 << 6,
    Groggy         = 1 << 7,
    Knockdown      = 1 << 8,
};

ENUM_CLASS_FLAGS(ERVCombatState);

/** Whether the heavy attack was held to max charge (auto-release) or released manually. */
UENUM(BlueprintType)
enum class ERVHeavyAttackTier : uint8
{
    Manual      = 0,
    AutoRelease = 1,
};

/**
 * Broadcast when any combat state bit is added.
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FRVOnCombatStateChanged, ERVCombatState);

/**
 * Broadcast by ForceEndAllActions.
 * Action components subscribe to self-clean their own state atomically.
 */
DECLARE_MULTICAST_DELEGATE(FRVOnForceEnd);

/**
 * Central authority for combat action states.
 * Owns ERVCombatState bitmask and invincibility flag.
 */
UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVCombatStateComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVCombatStateComponent();

    // --- Delegates -----------------------------------------------------------

    /** Broadcast whenever a state bit is added. Subscribers self-manage side effects. */
    FRVOnCombatStateChanged OnStateChanged;

    /** Broadcast by ForceEndAllActions. Action components subscribe to self-clean. */
    FRVOnForceEnd OnForceEnd;

    // --- Attack Trace --------------------------------------------------------

    void OpenHitWindow();
    void CloseHitWindow();

    /** Capsule overlap from WeaponRoot to WeaponTip. Damage resolved by current state. */
    void PerformAttackTrace();

    // --- State Control -------------------------------------------------------

    /**
     * Broadcasts OnForceEnd so all subscribed action components self-clean.
     * Called by URVHitReactionComponent when a poise-depleting hit lands.
     * HitReaction / Groggy / Knockdown states are NOT cleared here —
     * they are managed exclusively by URVHitReactionComponent.
     */
    void ForceEndAllActions();

    // --- State Queries -------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "RV|Combat")
    bool IsInState(ERVCombatState InState) const { return (CurrentStates & InState) != ERVCombatState::None; }

    UFUNCTION(BlueprintCallable, Category = "RV|Combat")
    bool IsInvincible() const { return bIsInvincible; }

    UFUNCTION(BlueprintCallable, Category = "RV|Combat")
    ERVHeavyAttackTier GetHeavyAttackTier() const { return ActiveTier; }

    UFUNCTION(BlueprintCallable, Category = "RV|Combat")
    bool IsGrounded() const;

    ERVCombatState GetActiveStates() const { return CurrentStates; }

    /**
     * Returns true if no blocking combat state is currently active.
     * InCoexistableStates: states permitted to coexist with the requested action.
     *
     * Blocking set includes HitReaction, Groggy, Knockdown — all prevent action input.
     */
    bool CheckAvailableState(ERVCombatState InCoexistableStates = ERVCombatState::None) const;

    // --- State Mutators (action components and ARVCharacterBase only) ---------

    /** Adds state bits and broadcasts OnStateChanged. */
    void AddState(ERVCombatState InState);

    FORCEINLINE void RemoveState(ERVCombatState InState) { CurrentStates &= ~InState; }
    FORCEINLINE bool HasState(ERVCombatState InState) const { return (CurrentStates & InState) != ERVCombatState::None; }

    void SetInvincible(bool bInInvincible) { bIsInvincible = bInInvincible; }
    void SetHeavyAttackTier(ERVHeavyAttackTier InTier) { ActiveTier = InTier; }

    // --- Reference Injection (called by ARVCharacterBase::BeginPlay) ----------

    void InitReferences(ACharacter* InOwnerCharacter,
                        URVEquipmentComponent* InEquipmentComponent,
                        UCharacterMovementComponent* InMovementComponent);

    // --- Attack State Handlers (wired by ARVCharacterBase to combo delegates) -

    /** Called when an attack action begins. Sets Attacking bit; clears Guarding if active. */
    void OnAttackStarted();

    /** Called when an attack action ends. Clears Attacking bit. */
    void OnAttackEnded();

protected:
    virtual void BeginPlay() override;

private:
    // --- Cached References ---------------------------------------------------

    UPROPERTY()
    TObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<URVEquipmentComponent> EquipmentComponent;

    UPROPERTY()
    TObjectPtr<UCharacterMovementComponent> MovementComponent;

    // --- Combat State --------------------------------------------------------

    ERVCombatState CurrentStates = ERVCombatState::None;
    bool bIsInvincible = false;

    // --- Heavy Attack Tier ---------------------------------------------------

    // Owned here because ResolveDamage reads it alongside HasState(HeavyAttacking).
    ERVHeavyAttackTier ActiveTier = ERVHeavyAttackTier::Manual;

    // --- Hit Window ----------------------------------------------------------

    TSet<TWeakObjectPtr<AActor>> HitActors;

    // --- Damage Resolution ---------------------------------------------------

    float ResolveDamage(const URVWeaponDataAsset* InWeaponData) const;
};