// Source/Revenant/Data/RVWeaponStatRow.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RVWeaponStatRow.generated.h"

/**
 * Per-weapon base stats stored in DT_WeaponStats.csv.
 * Attack values are computed as: base × multiplier (from FRVWeaponAttackStatRow).
 * Keeping base stats here means two weapons sharing the same animation style
 * can still have different damage output.
 *
 * HeavyChargeStaminaCost is a flat value — not multiplied.
 * It belongs to the charge action, not to any individual hit.
 */
USTRUCT(BlueprintType)
struct REVENANT_API FRVWeaponStatRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BasePoiseDamage;

	// Multiplied against FRVWeaponAttackStatRow.StaminaCostMultiplier per combo hit.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BaseStaminaCost;

	// Flat cost consumed at charge start — not used in per-hit multiplier calculation.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float HeavyChargeStaminaCost;
};