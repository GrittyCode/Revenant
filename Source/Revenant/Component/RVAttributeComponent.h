#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVAttributeComponent.generated.h"

struct FRVCharacterStatRow;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRVOnHealthChanged,  float, NewHealth,  float, InDelta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRVOnStaminaChanged, float, NewStamina, float, InDelta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FRVOnPoiseChanged,   float, NewPoiseRatio);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRVOnStaminaDepleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRVOnDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRVOnPoiseDepleted);

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVAttributeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVAttributeComponent();

    //--- Delegates -----------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "RV|Attribute")
    FRVOnHealthChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "RV|Attribute")
    FRVOnStaminaChanged OnStaminaChanged;

    UPROPERTY(BlueprintAssignable, Category = "RV|Attribute")
    FRVOnPoiseChanged OnPoiseChanged;

    UPROPERTY(BlueprintAssignable, Category = "RV|Attribute")
    FRVOnStaminaDepleted OnStaminaDepleted;

    UPROPERTY(BlueprintAssignable, Category = "RV|Attribute")
    FRVOnDeath OnDeath;

    UPROPERTY(BlueprintAssignable, Category = "RV|Attribute")
    FRVOnPoiseDepleted OnPoiseDepleted;

    //--- Init ----------------------------------------------------------------

    void InitFromStatRow(const FRVCharacterStatRow& InRow);

    //--- HP ------------------------------------------------------------------

    bool ApplyDamage(AActor* InInstigator, float InDamageAmount);
    bool ApplyHealing(float InHealAmount);

    UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
    bool IsAlive() const;

    //--- Stamina -------------------------------------------------------------

    bool ConsumeStamina(float InAmount);
    bool ApplyStaminaDamage(float InAmount);

    void ResetStaminaRegenDelay();
    void PauseStaminaRegen();
    void ResumeStaminaRegen();

    UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
    float GetStaminaPercent() const;

    UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
    float GetCurrentStamina() const { return CurrentStamina; }

    //--- Poise ---------------------------------------------------------------

    bool  ApplyPoiseDamage(float InPoiseDamage);
    void  ResetPoise();

    UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
    float GetMaxPoise()   const { return MaxPoise; }

    UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
    float GetPoiseRatio() const { return MaxPoise > 0.f ? CurrentPoise / MaxPoise : 0.f; }

protected:
    virtual void BeginPlay() override;

private:
    //--- HP ------------------------------------------------------------------

    UPROPERTY(VisibleAnywhere, Category = "RV|Attribute")
    float MaxHealth = 100.f;

    UPROPERTY(VisibleAnywhere, Category = "RV|Attribute")
    float CurrentHealth = 0.f;

    //--- Stamina -------------------------------------------------------------

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

    //--- Poise ---------------------------------------------------------------

    UPROPERTY(VisibleAnywhere, Category = "RV|Attribute")
    float MaxPoise = 100.f;

    UPROPERTY(VisibleAnywhere, Category = "RV|Attribute")
    float CurrentPoise = 0.f;

    float PoiseRegenDelay = 3.f;

    FTimerHandle PoiseRegenDelayHandle;

    void StartPoiseRegen();
};