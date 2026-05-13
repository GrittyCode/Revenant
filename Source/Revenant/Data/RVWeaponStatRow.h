// Source/Revenant/Data/RVWeaponStatRow.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RVWeaponStatRow.generated.h"

/**
 * Per-weapon base stats stored in DT_WeaponStats.csv.
 * Attack values are computed as: base × multiplier (from FRVAttackActionMultiplierRow).
 * Keeping base stats here means two weapons sharing the same animation style
 * can still have different damage output.
 */
USTRUCT(BlueprintType)
struct REVENANT_API FRVWeaponStatRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BasePoiseDamage;

	// Multiplied against FRVAttackActionMultiplierRow.StaminaCostMultiplier per action.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BaseStaminaCost;
};