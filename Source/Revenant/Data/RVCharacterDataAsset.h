#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RVCharacterDataAsset.generated.h"

UCLASS()
class REVENANT_API URVCharacterDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	//--- Attribute -----------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Attribute")
	float MaxHealth = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Attribute")
	float MaxStamina = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Attribute")
	float StaminaRegenRate = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Attribute")
	float StaminaRegenDelay = 1.5f;

	//--- Poise ---------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Poise")
	float MaxPoise = 100.f;

	// Consecutive stagger count before Groggy triggers instead.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Poise",
			  meta = (ClampMin = "1"))
	int32 GroggyThreshold = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Poise")
	float StaggerDuration = 0.5f;

	// Timer-driven — montage length does not define the window.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Poise")
	float GroggyDuration = 3.f;
};