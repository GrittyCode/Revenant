#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RVAttackActionMultiplierRow.generated.h"

/**
 * Per-hit multipliers stored in DT_AttackStats.csv.
 * Final value = FRVWeaponStatRow.Base* × Multiplier.
 */
USTRUCT(BlueprintType)
struct REVENANT_API FRVAttackActionMultiplierRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	float DamageMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly)
	float PoiseDamageMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly)
	float StaminaCostMultiplier = 0.f;
};