#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVCombatComponent.generated.h"

class ACharacter;
class URVAttributeComponent;
class URVEquipmentComponent;
class URVComboComponent;
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
	Manual      = 0,  // player released before MaxChargeTime
	AutoRelease = 1,  // held until MaxChargeTime — maximum damage
};

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URVCombatComponent();

	// --- Attack Trace -----------------------------------------------------------

	void OpenHitWindow();
	void CloseHitWindow();

	/** Capsule overlap from WeaponRoot to WeaponTip socket. Damage resolved internally by current state. */
	void PerformAttackTrace();

	// --- Combo ------------------------------------------------------------------

	void TryStartCombo();

	// --- Heavy Attack -----------------------------------------------------------

	/**
	 * Begins heavy attack charge. Plays HeavyChargeMontage (loops until released).
	 * Consumes stamina immediately — charge cancellation is a penalty.
	 * Called by ARVCharacterPlayer::InputHeavyAttackStarted().
	 */
	void StartHeavyAttack();

	/**
	 * Requests heavy attack release.
	 * If the charge montage has not yet reached the Loop section, the release is
	 * buffered in bPendingRelease and fired automatically when Loop entry is confirmed.
	 * Called by ARVCharacterPlayer::InputHeavyAttackCompleted().
	 */
	void ReleaseHeavyAttack();

	/**
	 * Called by AnimNotify_HeavyAttackReady on NotifyBegin (Loop section entry).
	 * Activates release gating and flushes any pending buffered release.
	 */
	void SetHeavyAttackReady(bool bReady);

	// --- Dodge ------------------------------------------------------------------

	void StartDodge(const FVector& InDodgeDirection);
	void SetDodgeIFrame(bool bActivate);

	// --- Guard ------------------------------------------------------------------

	void StartGuard();
	void EndGuard();
	void HandleGuardHit(float InDamageAmount);

	// --- Sprint -----------------------------------------------------------------

	void StartSprint();
	void EndSprint();

	// --- State Control ----------------------------------------------------------

	/**
	 * Forcibly terminates any active combat action.
	 * Called by the hit-reaction system (Phase 3) to cancel in-progress actions.
	 */
	void ForceEndAllActions();

	// --- State Queries ----------------------------------------------------------

	/**
	 * Returns true if all bits in InState are currently active.
	 * Supports compound queries: IsInState(Attacking | HeavyCharging)
	 */
	UFUNCTION(BlueprintCallable, Category = "RV|Combat")
	bool IsInState(ERVCombatState InState) const { return (CurrentStates & InState) != ERVCombatState::None; }

	// Invincible and Sprinting are modifier flags outside the combat state enum.
	UFUNCTION(BlueprintCallable, Category = "RV|Combat")
	bool IsInvincible() const { return bIsInvincible; }

	UFUNCTION(BlueprintCallable, Category = "RV|Combat")
	bool IsSprinting()  const { return bIsSprinting; }

	UFUNCTION(BlueprintCallable, Category = "RV|Combat")
	ERVHeavyAttackTier GetHeavyAttackTier() const { return ActiveTier; }

	UFUNCTION(BlueprintCallable, Category = "RV|Combat")
	bool IsGrounded() const;

	/** Returns the active combat state bitmask directly. */
	ERVCombatState GetActiveStates() const { return CurrentStates; }

	/**
	 * Returns true if no blocking combat state is currently active.
	 * InCoexistableStates: states that are allowed to be active simultaneously (e.g. Guarding while jumping).
	 */
	bool CheckAvailableState(ERVCombatState InCoexistableStates = ERVCombatState::None) const;

protected:
	virtual void BeginPlay() override;

private:
	// --- Cached References --------------------------------------------

	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;


	UPROPERTY()
	TObjectPtr<URVAttributeComponent> AttributeComponent;

	UPROPERTY()
	TObjectPtr<URVEquipmentComponent> EquipmentComponent;

	UPROPERTY()
	TObjectPtr<URVComboComponent> ComboComponent;

	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> MovementComponent;

	// --- Hit Window -------------------------------------------------------------

	TSet<TWeakObjectPtr<AActor>> HitActors;

	// --- Combat State -----------------------------------------------------------

	// Single bitmask owning all combat states. Use AddState / RemoveState / HasState to modify.
	ERVCombatState CurrentStates = ERVCombatState::None;

	// Invincible: dodge sub-flag — not a combat state, does not block actions.
	bool bIsInvincible = false;

	// Sprinting: movement modifier — not a combat state, does not block actions.
	bool bIsSprinting  = false;

	// --- Heavy Attack -----------------------------------------------------------

	/** Hold time at which charge auto-releases at maximum damage. */
	UPROPERTY(EditDefaultsOnly, Category = "RV|HeavyAttack")
	float MaxChargeTime = 1.5f;

	// Tier resolved at ExecuteHeavyAttack — read by ResolveDamage for damage lookup.
	ERVHeavyAttackTier ActiveTier = ERVHeavyAttackTier::Manual;

	// Fires ReleaseHeavyAttack automatically after MaxChargeTime.
	FTimerHandle ChargeAutoReleaseHandle;

	// True once AnimNotify_HeavyAttackReady fires (Loop section entered).
	bool bCanHeavyRelease = false;

	// Set when ReleaseHeavyAttack is called before bCanHeavyRelease is true.
	bool bPendingRelease  = false;

	// Set by OnChargeAutoRelease before calling ReleaseHeavyAttack.
	// Tells ExecuteHeavyAttack to use AutoRelease tier (max damage).
	bool bIsAutoRelease   = false;

	// --- Sprint -----------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "RV|Sprint")
	float SprintSpeed = 1000.f;

	float OriginalWalkSpeed = 0.f;

	// --- Guard Break Recovery ---------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "RV|Guard")
	float GuardBreakRecoveryTime = 2.f;

	FTimerHandle GuardBreakRecoveryHandle;

	// --- State Management Helpers -----------------------------------------------

	FORCEINLINE void AddState(ERVCombatState InState)          { CurrentStates |= InState; }
	FORCEINLINE void RemoveState(ERVCombatState InState)       { CurrentStates &= ~InState; }
	FORCEINLINE bool HasState(ERVCombatState InState)    const { return (CurrentStates & InState) != ERVCombatState::None; }

	// --- Damage Resolution ------------------------------------------------------

	/** Selects the correct damage value from WeaponData based on the current attack state. */
	float ResolveDamage(const URVWeaponDataAsset* InWeaponData) const;

	// --- Internal Helpers -------------------------------------------------------

	void EndDodge();
	void EndHeavyAttack();

	/** Performs the actual montage transition from charge to release. Called by ReleaseHeavyAttack and SetHeavyAttackReady. */
	void ExecuteHeavyAttack();

	void OnComboStartedHandler();
	void OnComboEndedHandler();

	UFUNCTION()
	void OnGuardBreakHandler();

	UFUNCTION()
	void OnGuardBreakRecoveryComplete();

	UFUNCTION()
	void OnChargeAutoRelease();

	void OnDodgeMontageBlendingOut(UAnimMontage*, bool bInterrupted);
	void OnChargeMontageBlendingOut(UAnimMontage*, bool bInterrupted);
	void OnReleaseMontageBlendingOut(UAnimMontage*, bool bInterrupted);
};