#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVVitalComponent.generated.h"

struct FRVCharacterStatRow;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRVOnHealthChanged, float, NewHealthRatio);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRVOnPoiseChanged,  float, NewPoiseRatio);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRVOnDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRVOnPoiseDepleted);

// Owns Health and Poise for any character (player and boss).
// Stamina is player-only — lives in URVStaminaComponent on ARVCharacterPlayer.
UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVVitalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVVitalComponent();

    //--- Delegates -----------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "RV|Vital")
    FRVOnHealthChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "RV|Vital")
    FRVOnPoiseChanged OnPoiseChanged;

    UPROPERTY(BlueprintAssignable, Category = "RV|Vital")
    FRVOnDeath OnDeath;

    UPROPERTY(BlueprintAssignable, Category = "RV|Vital")
    FRVOnPoiseDepleted OnPoiseDepleted;

    //--- Init ----------------------------------------------------------------

    void InitFromStatRow(const FRVCharacterStatRow& InRow);

    //--- Health --------------------------------------------------------------

    bool ApplyDamage(AActor* InInstigator, float InDamageAmount);
    bool ApplyHealing(float InHealAmount);
	
	float GetHealthPercent() const;
	bool IsAlive() const { return CurrentHealth > 0.f; }

    //--- Poise ---------------------------------------------------------------

    void  ApplyPoiseDamage(float InPoiseDamage);
    void  ResetPoise();
	float GetMaxPoise()   const { return MaxPoise; }
	float GetPoiseRatio() const { return MaxPoise > 0.f ? CurrentPoise / MaxPoise : 0.f; }

	
private:
	
    //--- Health --------------------------------------------------------------

    UPROPERTY(VisibleAnywhere, Category = "RV|Vital")
    float MaxHealth = 100.f;

    UPROPERTY(VisibleAnywhere, Category = "RV|Vital")
    float CurrentHealth = 0.f;

    //--- Poise ---------------------------------------------------------------

    UPROPERTY(VisibleAnywhere, Category = "RV|Vital")
    float MaxPoise = 100.f;

    UPROPERTY(VisibleAnywhere, Category = "RV|Vital")
    float CurrentPoise = 0.f;

    float PoiseRegenDelay = 3.f;

    FTimerHandle PoiseRegenDelayHandle;

    void StartPoiseRegen();
};
