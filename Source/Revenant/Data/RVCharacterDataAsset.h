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

	//--- Stamina Costs -------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Stamina")
	float DodgeStaminaCost = 30.f;

	//--- Poise ---------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Poise")
	float MaxPoise = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Poise")
	float StaggerDuration = 0.5f;
};