#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVCombatComponent.generated.h"

class URVAttributeComponent;
class URVEquipmentComponent;
class URVComboComponent;
class UCharacterMovementComponent;
class UAnimMontage;

/** Bitflag enum representing mutually-exclusive-but-composable combat states.
 *  Used by CanPerformAction() to gate action entry. */
UENUM(BlueprintType, meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class ERVCombatState : uint8
{
    None        = 0       UMETA(Hidden),
    Attacking   = 1 << 0  UMETA(DisplayName="Attacking"),
    Dodging     = 1 << 1  UMETA(DisplayName="Dodging"),
    Guarding    = 1 << 2  UMETA(DisplayName="Guarding"),
    GuardBroken = 1 << 3  UMETA(DisplayName="GuardBroken"),
};

ENUM_CLASS_FLAGS(ERVCombatState);

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVCombatComponent();

	// --- Attack Trace -----------------------------------------------------------

	/** Opens hit window. Clears HitActors so each swing hits each target only once. */
	void OpenHitWindow();

	/** Closes hit window. Clears HitActors. */
	void CloseHitWindow();

	/**
	 * Capsule overlap from WeaponRoot to WeaponTip socket.
	 * Calls IRVDamageable::ApplyDamage on every unique hit actor.
	 * Invoked by UAnimNotifyState_AttackHitCheck::NotifyTick().
	 */
	void PerformAttackTrace();

    // --- Combo ------------------------------------------------------------------

    /**
     * Gates combo entry (grounded + CanPerformAction) then delegates to URVComboComponent.
     * Called by ARVCharacterPlayer::InputAttack().
     */
    void TryStartCombo();

    // --- Dodge ------------------------------------------------------------------

    /**
     * Starts a directional dodge. Blocked while airborne.
     * Cancels sprint and guard on entry.
     * InDodgeDirection: world-space unit vector.
     */
    void StartDodge(const FVector& InDodgeDirection);

    /** Called by AnimNotify_DodgeIFrame to open/close the i-frame window. */
    void SetDodgeIFrame(bool bActivate);

    // --- Guard ------------------------------------------------------------------

    /** Starts guard. Blocked while airborne. Cancels sprint on entry. */
    void StartGuard();
    void EndGuard();

    /**
     * Called by ARVCharacterBase::ApplyDamage when a hit lands while guarding.
     * Applies stamina damage and plays GuardHit montage if guard holds.
     * If stamina reaches 0, OnGuardBreak fires — OnGuardBreakHandler takes over.
     */
    void HandleGuardHit(float InDamageAmount);

    // --- Sprint -----------------------------------------------------------------

    /**
     * Raises MaxWalkSpeed to SprintSpeed.
     * Blocked while airborne, dodging, guarding, or guard-broken.
     * Caches original MaxWalkSpeed for exact restoration on EndSprint.
     */
    void StartSprint();

    /** Restores MaxWalkSpeed to the value cached at StartSprint entry. */
    void EndSprint();

    // --- State Write (called by URVComboComponent delegate) --------------------

    /** Bound to URVComboComponent::OnComboStarted / OnComboEnded. */
    void SetAttacking(bool bInIsAttacking);

    // --- State Queries ----------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "RV|Combat")
    bool IsAttacking()   const { return bIsAttacking; }

    UFUNCTION(BlueprintCallable, Category = "RV|Combat")
    bool IsDodging()     const { return bIsDodging; }

    UFUNCTION(BlueprintCallable, Category = "RV|Combat")
    bool IsGuarding()    const { return bIsGuarding; }

    UFUNCTION(BlueprintCallable, Category = "RV|Combat")
    bool IsInvincible()  const { return bIsInvincible; }

    UFUNCTION(BlueprintCallable, Category = "RV|Combat")
    bool IsGuardBroken() const { return bIsGuardBroken; }

    UFUNCTION(BlueprintCallable, Category = "RV|Combat")
    bool IsSprinting()   const { return bIsSprinting; }

    /** Returns true if the owner is on the ground. Single source of IsFalling() knowledge. */
    UFUNCTION(BlueprintCallable, Category = "RV|Combat")
    bool IsGrounded() const;

    /** Returns a bitmask of currently active blocking states. */
    ERVCombatState GetActiveStates() const;

    /**
     * Returns true if no blocking state is active, ignoring states in InAllowedActiveStates.
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

    UPROPERTY()
    TObjectPtr<URVComboComponent> ComboComponent;

    UPROPERTY()
    TObjectPtr<UCharacterMovementComponent> MovementComponent;

	// --- Hit Window -------------------------------------------------------------

	// Populated during hit window (OpenHitWindow ~ CloseHitWindow).
	// Prevents the same actor from being hit multiple times per swing.
	TSet<TWeakObjectPtr<AActor>> HitActors;
	
    // --- State ------------------------------------------------------------------

    bool bIsAttacking   = false;
    bool bIsDodging     = false;
    bool bIsGuarding    = false;
    bool bIsInvincible  = false;
    bool bIsGuardBroken = false;
    bool bIsSprinting   = false;

    // --- Sprint -----------------------------------------------------------------

    /** Target MaxWalkSpeed while sprinting. */
    UPROPERTY(EditDefaultsOnly, Category = "RV|Sprint")
    float SprintSpeed = 1000.f;

    // Cached at StartSprint — restored exactly on EndSprint regardless of BP-set WalkSpeed.
    float OriginalWalkSpeed = 0.f;

    // --- Guard Break Recovery ---------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "RV|Guard")
    float GuardBreakRecoveryTime = 2.f;

    FTimerHandle GuardBreakRecoveryHandle;

    // --- Internal Helpers -------------------------------------------------------

    void EndDodge();

    // Bound to URVComboComponent::OnComboStarted / OnComboEnded (non-dynamic delegate)
    void OnComboStartedHandler();
    void OnComboEndedHandler();

    UFUNCTION()
    void OnGuardBreakHandler();

    UFUNCTION()
    void OnGuardBreakRecoveryComplete();

    void OnDodgeMontageBlendingOut(UAnimMontage*, bool);
};