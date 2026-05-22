// Source/Revenant/Component/RVAttributeComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVAttributeComponent.generated.h"

class URVCharacterDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRVOnHealthChanged,  float, NewHealth,  float, InDelta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRVOnStaminaChanged, float, NewStamina, float, InDelta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FRVOnPoiseChanged,   float, NewPoiseRatio);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRVOnStaminaDepleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRVOnDeath);

/**
 * Fired when poise reaches 0 via ApplyPoiseDamage.
 * The stagger/groggy/knockdown decision is made synchronously inside
 * URVHitReactionComponent::HandleHit using the bool return value.
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
    FRVOnStaminaChanged OnStaminaChanged;

    // Fired on every poise change. NewPoiseRatio = CurrentPoise / MaxPoise (0..1).
    UPROPERTY(BlueprintAssignable, Category = "RV|Attribute")
    FRVOnPoiseChanged OnPoiseChanged;

    /** Fired when stamina reaches 0 via ApplyStaminaDamage. URVGuardComponent binds this. */
    UPROPERTY(BlueprintAssignable, Category = "RV|Attribute")
    FRVOnStaminaDepleted OnStaminaDepleted;

    UPROPERTY(BlueprintAssignable, Category = "RV|Attribute")
    FRVOnDeath OnDeath;

    /**
     * Fired when poise reaches 0 via ApplyPoiseDamage.
     * For external subscribers only — reaction logic is handled synchronously.
     */
    UPROPERTY(BlueprintAssignable, Category = "RV|Attribute")
    FRVOnPoiseDepleted OnPoiseDepleted;

    // ─── Init ────────────────────────────────────────────────────────────────

    void InitFromDataAsset(URVCharacterDataAsset* InData);
    void InitFromValues(float InMaxHP, float InMaxPoise);

    // ─── HP ──────────────────────────────────────────────────────────────────

    /** Returns true if the character survived (HP > 0 after hit). */
    bool ApplyDamage(AActor* InInstigator, float InDamageAmount);
    bool ApplyHealing(float InHealAmount);

    UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
    bool IsAlive() const;

    // ─── Stamina ─────────────────────────────────────────────────────────────

    bool ConsumeStamina(float InAmount);

    /** Returns true if guard held (stamina still > 0). */
    bool ApplyStaminaDamage(float InAmount);

    void ResetStaminaRegenDelay();

    /** Hard-stops regen. Reserved for heavy charge suppression. */
    void PauseStaminaRegen();

    /** Schedules regen to resume after StaminaRegenDelay. */
    void ResumeStaminaRegen();

    UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
    float GetStaminaPercent() const;

    UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
    float GetCurrentStamina() const { return CurrentStamina; }

    // ─── Poise ───────────────────────────────────────────────────────────────

    /**
     * Reduces poise by InPoiseDamage.
     * Returns true if poise was depleted (reached 0).
     */
    bool ApplyPoiseDamage(float InPoiseDamage);

    /**
     * Restores poise to MaxPoise.
     * Called by ARVSevarogCharacter after Groggy ends.
     */
    void ResetPoise();

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