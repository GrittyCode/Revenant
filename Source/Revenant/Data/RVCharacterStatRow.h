#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RVCharacterStatRow.generated.h"

USTRUCT(BlueprintType)
struct REVENANT_API FRVCharacterStatRow : public FTableRowBase
{
	GENERATED_BODY()

	//--- Attribute -----------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxHealth = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxPoise = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxStamina = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float StaminaRegenRate = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float StaminaRegenDelay = 1.5f;

	//--- Movement ------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MoveSpeed = 400.f;

	//--- Hit Reaction --------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float StaggerDuration = 0.5f;

	// CurrentPoise / MaxPoise drops to or below this ratio → Stagger.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StaggerThreshold = 0.5f;

	// Single-hit PoiseDamage / MaxPoise reaches or exceeds this ratio → Knockdown.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float KnockdownThreshold = 0.4f;

	// Seconds after the last poise hit before poise begins recovering.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float PoiseRegenDelay = 3.f;
};