#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/RVWeaponDataAsset.h"
#include "RVCharacterDataAsset.generated.h"

UCLASS()
class REVENANT_API URVCharacterDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Attribute")
	float MaxHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Attribute")
	float MaxStamina = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Attribute")
	float StaminaRegenRate = 20.f;

	/** Seconds before stamina regen begins after an action. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Attribute")
	float StaminaRegenDelay = 1.5f;
};