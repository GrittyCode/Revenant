#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVCombatStateComponent.generated.h"

class ACharacter;
class URVEquipmentComponent;
class UCharacterMovementComponent;
class UAnimMontage;
class URVWeaponDataAsset;

UENUM(BlueprintType, meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class ERVCombatState : uint8
{
    None           = 0        UMETA(Hidden),
    Attacking      = 1 << 0   UMETA(DisplayName="Attacking"),
    HeavyAttacking = 1 << 1   UMETA(DisplayName="HeavyAttacking"),
    Dodging        = 1 << 2   UMETA(DisplayName="Dodging"),
    Guarding       = 1 << 3   UMETA(DisplayName="Guarding"),
    GuardBroken    = 1 << 4   UMETA(DisplayName="GuardBroken"),
    HeavyCharging  = 1 << 5   UMETA(DisplayName="HeavyCharging"),
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
 * Subscribers (e.g. URVSprintComponent) react to state transitions without
 * being called directly by action components.
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
 * Sprint is managed independently by URVSprintComponent.
 * Only action components and ARVCharacterBase should call AddState / RemoveState directly.
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
     * Called by the hit-reaction system (Phase 3).
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

    /** Selects damage value from WeaponData based on current attack state and tier. */
    float ResolveDamage(const URVWeaponDataAsset* InWeaponData) const;
};