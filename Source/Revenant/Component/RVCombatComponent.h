// Source/Revenant/Component/RVCombatComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVCombatComponent.generated.h"

class URVAttributeComponent;
class URVEquipmentComponent;
class UAnimMontage;

/** Bitflag enum representing mutually-exclusive-but-composable combat states.
 *  Used by CanPerformAction() to gate action entry. */
UENUM(BlueprintType, meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class ERVCombatState : uint8
{
	None = 0 UMETA(Hidden),
	Attacking = 1 << 0 UMETA(DisplayName="Attacking"),
	Dodging = 1 << 1 UMETA(DisplayName="Dodging"),
	Guarding = 1 << 2 UMETA(DisplayName="Guarding"),
	GuardBroken = 1 << 3 UMETA(DisplayName="GuardBroken"),
};

ENUM_CLASS_FLAGS(ERVCombatState);

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URVCombatComponent();

	// --- Attack Trace -----------------------------------------------------------

	/**
	 * Capsule overlap from WeaponRoot to WeaponTip socket.
	 * Calls IRVDamageable::ApplyDamage on every unique hit actor.
	 * Invoked by ARVCharacterBase::ActivateHitCheck().
	 */
	void PerformAttackTrace();

	// --- Dodge ------------------------------------------------------------------

	/**
	 * Starts a directional dodge.
	 * InDodgeDirection: world-space unit vector.
	 */
	void StartDodge(const FVector& InDodgeDirection);

	/** Called by AnimNotify_DodgeIFrame to open/close the i-frame window. */
	void SetDodgeIFrame(bool bActivate);

	// --- Guard ------------------------------------------------------------------

	void StartGuard();
	void EndGuard();

	/**
	 * Called by ARVCharacterBase::ApplyDamage when a hit lands while guarding.
	 * Applies stamina damage and plays GuardHit montage if guard holds.
	 * If stamina reaches 0, OnGuardBreak fires — OnGuardBreakHandler takes over.
	 */
	void HandleGuardHit(float InDamageAmount);

	// --- State Write (called by URVComboComponent) ------------------------------

	/** URVComboComponent calls this on combo start / end. */
	void SetAttacking(bool bInIsAttacking);

	// --- State Queries ----------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "RV|Combat")
	bool IsAttacking() const { return bIsAttacking; }

	UFUNCTION(BlueprintCallable, Category = "RV|Combat")
	bool IsDodging() const { return bIsDodging; }

	UFUNCTION(BlueprintCallable, Category = "RV|Combat")
	bool IsGuarding() const { return bIsGuarding; }

	UFUNCTION(BlueprintCallable, Category = "RV|Combat")
	bool IsInvincible() const { return bIsInvincible; }

	UFUNCTION(BlueprintCallable, Category = "RV|Combat")
	bool IsGuardBroken() const { return bIsGuardBroken; }

	/** Returns a bitmask of currently active blocking states. */
	ERVCombatState GetActiveStates() const;

	/**
	 * Returns true if no blocking state is active, ignoring states in InIgnoredStates.
	 *
	 * Example:
	 *   StartDodge: CanPerformAction(ERVCombatState::Guarding)
	 *     -- Guard is excluded because dodge auto-cancels it on entry.
	 *   StartGuard: CanPerformAction()
	 *     -- All blocking states checked.
	 */
	bool CanPerformAction(ERVCombatState InAllowedActiveStates = ERVCombatState::None) const;

protected:
	virtual void BeginPlay() override;

private:
	// --- Cached Component References --------------------------------------------

	UPROPERTY()
	TObjectPtr<URVAttributeComponent> AttributeComponent;

	UPROPERTY()
	TObjectPtr<URVEquipmentComponent> EquipmentComponent;

	// --- State ------------------------------------------------------------------

	bool bIsAttacking = false;
	bool bIsDodging = false;
	bool bIsGuarding = false;
	bool bIsInvincible = false;
	bool bIsGuardBroken = false;

	// --- Guard Break Recovery ---------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "RV|Guard")
	float GuardBreakRecoveryTime = 2.f;

	FTimerHandle GuardBreakRecoveryHandle;

	// --- Internal Helpers -------------------------------------------------------

	void EndDodge();
	
	UFUNCTION()
	void OnGuardBreakHandler();

	UFUNCTION()
	void OnGuardBreakRecoveryComplete();
	
	void OnDodgeMontageBlendingOut(UAnimMontage*, bool);
};