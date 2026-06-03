#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVStaminaComponent.generated.h"

struct FRVPlayerStatRow;

DECLARE_MULTICAST_DELEGATE_OneParam(FRVOnStaminaChanged, float);
DECLARE_MULTICAST_DELEGATE(FRVOnStaminaDepleted);

UCLASS(ClassGroup=(Revenant))
class REVENANT_API URVStaminaComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVStaminaComponent();

    //--- Delegates -----------------------------------------------------------
	
	FRVOnStaminaChanged OnStaminaChanged;
    FRVOnStaminaDepleted OnStaminaDepleted;

    //--- Init ----------------------------------------------------------------

    void InitFromStatRow(const FRVPlayerStatRow& InRow);

    //--- Stamina -------------------------------------------------------------

    // Returns true if stamina was available and consumed.
    bool ConsumeStamina(float InAmount);

    // Guard hit path: consumes stamina, returns true if guard holds.
    bool ApplyStaminaDamage(float InAmount);

    float GetStaminaPercent() const;
    float GetCurrentStamina() const { return CurrentStamina; }

    //--- Regen control -------------------------------------------------------

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