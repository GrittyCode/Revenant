#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVAttributeComponent.generated.h"

class URVCharacterDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRVOnHealthChanged,  float, NewHealth,  float, InDelta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRVOnDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRVOnStaminaChanged, float, NewStamina, float, InDelta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRVOnStaminaDepleted);

/**
 * Fired when poise reaches 0 via ApplyPoiseDamage.
 * The stagger/groggy/knockdown decision is made synchronously inside
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRVOnPoiseDepleted);

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVAttributeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVAttributeComponent();

    // ─── Delegates ───────────────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "RV|Attribute")
    FRVOnHealthChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "RV|Attribute")
    FRVOnDeath OnDeath;

    UPROPERTY(BlueprintAssignable, Category = "RV|Attribute")
    FRVOnStaminaChanged OnStaminaChanged;

    /** Fired when stamina reaches 0 via ApplyStaminaDamage. URVGuardComponent binds this. */
    UPROPERTY(BlueprintAssignable, Category = "RV|Attribute")
    FRVOnStaminaDepleted OnStaminaDepleted;

    /**
     * Fired when poise reaches 0 via ApplyPoiseDamage.
     * For external subscribers only — reaction logic is handled synchronously
     */
    UPROPERTY(BlueprintAssignable, Category = "RV|Attribute")
    FRVOnPoiseDepleted OnPoiseDepleted;

    // ─── Init ────────────────────────────────────────────────────────────────

    /** Called by ARVCharacterBase::BeginPlay after CharacterData is assigned. */
    void InitFromDataAsset(URVCharacterDataAsset* InData);

    // ─── HP ──────────────────────────────────────────────────────────────────

    /** Returns true if the character survived (HP > 0 after hit). */
    bool ApplyDamage(AActor* InInstigator, float InDamageAmount);
    bool ApplyHealing(float InHealAmount);

    UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
    bool IsAlive() const;

    // ─── Stamina ─────────────────────────────────────────────────────────────

    /**
     * Returns true if stamina was sufficient.
     */
    bool ConsumeStamina(float InAmount);

    /**
     * Returns true if guard held (stamina still > 0).
     */
    bool ApplyStaminaDamage(float InAmount);

    /** Stops regen. Called by action components on combat state entry. */
    void PauseStaminaRegen();

    /**
     * Schedules regen to resume after StaminaRegenDelay.
     * Called by action components on combat state exit.
     */
    void ResumeStaminaRegen();

    UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
    float GetStaminaPercent() const;

    UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
    float GetCurrentStamina() const { return CurrentStamina; }

    // ─── Poise ───────────────────────────────────────────────────────────────

    /**
     * Reduces poise by InPoiseDamage.
     * Returns true if poise was depleted (reached 0) — URVHitReactionComponent.
     */
    bool ApplyPoiseDamage(float InPoiseDamage);

    /**
     * Restores poise to MaxPoise.
     * Called by URVHitReactionComponent after each Stagger or Groggy entry
     */
    void ResetPoise();

    UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
    float GetPoisePercent() const;

    UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
    float GetCurrentPoise() const { return CurrentPoise; }

protected:
    virtual void BeginPlay() override;

private:
    // --- HP ------------------------------------------------------------------

    UPROPERTY(VisibleAnywhere, Category = "RV|Attribute")
    float MaxHealth = 100.f;

    UPROPERTY(VisibleAnywhere, Category = "RV|Attribute")
    float CurrentHealth = 0.f;

    // --- Stamina -------------------------------------------------------------

    UPROPERTY(VisibleAnywhere, Category = "RV|Attribute")
    float MaxStamina = 100.f;

    UPROPERTY(VisibleAnywhere, Category = "RV|Attribute")
    float CurrentStamina = 0.f;

    UPROPERTY(EditDefaultsOnly, Category = "RV|Attribute")
    float StaminaRegenRate = 10.f;

    UPROPERTY(EditDefaultsOnly, Category = "RV|Attribute")
    float StaminaRegenInterval = 0.1f;

    /** Delay before regen starts after ResumeStaminaRegen is called. Loaded from DataAsset. */
    UPROPERTY(VisibleAnywhere, Category = "RV|Attribute")
    float StaminaRegenDelay = 1.5f;

    FTimerHandle StaminaRegenDelayHandle;
    FTimerHandle StaminaRegenHandle;

    void StartStaminaRegenTick();
    void TickStaminaRegen();

    // --- Poise ---------------------------------------------------------------

    UPROPERTY(VisibleAnywhere, Category = "RV|Attribute")
    float MaxPoise = 100.f;

    UPROPERTY(VisibleAnywhere, Category = "RV|Attribute")
    float CurrentPoise = 0.f;
};