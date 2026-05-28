#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVStaminaComponent.generated.h"

struct FRVPlayerStatRow;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRVOnStaminaChanged, float, NewStamina, float, InDelta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRVOnStaminaDepleted);

// Player-only stamina — owned exclusively by ARVCharacterPlayer.
// Boss characters do not have this component.
UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVStaminaComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVStaminaComponent();

    //--- Delegates -----------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "RV|Stamina")
    FRVOnStaminaChanged OnStaminaChanged;

    UPROPERTY(BlueprintAssignable, Category = "RV|Stamina")
    FRVOnStaminaDepleted OnStaminaDepleted;

    //--- Init ----------------------------------------------------------------

    void InitFromStatRow(const FRVPlayerStatRow& InRow);

    //--- Stamina -------------------------------------------------------------

    // Returns true if stamina was available and consumed.
    bool ConsumeStamina(float InAmount);

    // Guard hit path: consumes stamina, returns true if guard holds.
    bool ApplyStaminaDamage(float InAmount);

    UFUNCTION(BlueprintCallable, Category = "RV|Stamina")
    float GetStaminaPercent() const;

    UFUNCTION(BlueprintCallable, Category = "RV|Stamina")
    float GetCurrentStamina() const { return CurrentStamina; }

    //--- Regen control -------------------------------------------------------

    // Call after any stamina-consuming action to restart the regen delay.
    void ResetStaminaRegenDelay();
    void PauseStaminaRegen();
    void ResumeStaminaRegen();

private:
    UPROPERTY(VisibleAnywhere, Category = "RV|Stamina")
    float MaxStamina = 100.f;

    UPROPERTY(VisibleAnywhere, Category = "RV|Stamina")
    float CurrentStamina = 0.f;

    UPROPERTY(EditDefaultsOnly, Category = "RV|Stamina")
    float StaminaRegenRate = 10.f;

    UPROPERTY(EditDefaultsOnly, Category = "RV|Stamina")
    float StaminaRegenInterval = 0.1f;

    UPROPERTY(EditDefaultsOnly, Category = "RV|Stamina")
    float StaminaRegenDelay = 1.5f;

    FTimerHandle StaminaRegenDelayHandle;
    FTimerHandle StaminaRegenHandle;

    void StartStaminaRegenTick();
    void TickStaminaRegen();
};
