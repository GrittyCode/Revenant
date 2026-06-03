#pragma once

#include "CoreMinimal.h"
#include "Data/Row/RVCharacterStatRow.h"
#include "RVPlayerStatRow.generated.h"

// Player-only stats stored in DT_PlayerStats.
// Extends FRVCharacterStatRow so InitFromStatRow(const FRVCharacterStatRow&) accepts this by base ref.
USTRUCT(BlueprintType)
struct REVENANT_API FRVPlayerStatRow : public FRVCharacterStatRow
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	float MaxStamina = 100.f;

	UPROPERTY(EditDefaultsOnly)
	float StaminaRegenRate = 20.f;

	UPROPERTY(EditDefaultsOnly)
	float StaminaRegenDelay = 1.5f;

	// Timer interval (seconds) between each stamina regen tick.
	UPROPERTY(EditDefaultsOnly)
	float StaminaRegenInterval = 0.1f;

	UPROPERTY(EditDefaultsOnly)
	float DodgeStaminaCost = 30.f;

	UPROPERTY(EditDefaultsOnly)
	float SprintSpeed = 1000.f;
};