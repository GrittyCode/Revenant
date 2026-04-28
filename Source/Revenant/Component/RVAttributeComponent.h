// Source/Revenant/Component/RVAttributeComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVAttributeComponent.generated.h"

class URVCharacterDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRVOnHealthChanged,  float, NewHealth,  float, InDelta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRVOnDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRVOnStaminaChanged, float, NewStamina, float, InDelta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRVOnGuardBreak);

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

    /** Fired when stamina reaches 0 while guarding. URVCombatComponent binds this. */
    UPROPERTY(BlueprintAssignable, Category = "RV|Attribute")
    FRVOnGuardBreak OnGuardBreak;

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
     * Reduces stamina for an action cost (attack, dodge).
     * Does NOT fire OnGuardBreak — see ApplyStaminaDamage for guard hits.
     * Returns true if stamina was sufficient.
     */
    bool ConsumeStamina(float InAmount);

    /**
     * Reduces stamina from a blocked hit.
     * Fires OnGuardBreak if stamina reaches 0.
     * Returns true if guard held (stamina still > 0).
     */
    bool ApplyStaminaDamage(float InAmount);

    /** Stops regen. Called by URVCombatComponent on combat state entry. */
    void PauseStaminaRegen();

    /**
     * Schedules regen to resume after StaminaRegenDelay.
     * Called by URVCombatComponent on combat state exit.
     */
    void ResumeStaminaRegen();

    UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
    float GetStaminaPercent() const;

    UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
    float GetCurrentStamina() const { return CurrentStamina; }


protected:
    virtual void BeginPlay() override;

private:
    // --- HP ---------------------------------------------------------

    UPROPERTY(VisibleAnywhere, Category = "RV|Attribute")
    float MaxHealth = 100.f;

    UPROPERTY(VisibleAnywhere, Category = "RV|Attribute")
    float CurrentHealth = 0.f;

    // --- Stamina ---------------------------------------------------------

    UPROPERTY(VisibleAnywhere, Category = "RV|Attribute")
    float MaxStamina = 100.f;

    UPROPERTY(VisibleAnywhere, Category = "RV|Attribute")
    float CurrentStamina = 0.f;

    /** Stamina recovered per regen tick. */
    UPROPERTY(EditDefaultsOnly, Category = "RV|Attribute")
    float StaminaRegenRate = 10.f;

    /** Seconds between regen ticks. */
    UPROPERTY(EditDefaultsOnly, Category = "RV|Attribute")
    float StaminaRegenInterval = 0.1f;

    /** Delay before regen starts after ResumeStaminaRegen is called. */
    UPROPERTY(EditDefaultsOnly, Category = "RV|Attribute")
    float StaminaRegenDelay = 1.5f;

    FTimerHandle StaminaRegenDelayHandle;
    FTimerHandle StaminaRegenHandle;

    void StartStaminaRegenTick();
    void TickStaminaRegen();
	
};