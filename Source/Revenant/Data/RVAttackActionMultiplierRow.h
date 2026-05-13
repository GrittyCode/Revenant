#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Interface/RVDamageable.h"
#include "RVAttackActionMultiplierRow.generated.h"

/**
 * Per-hit multipliers stored in DT_AttackStats.csv.
 * Final value = FRVWeaponStatRow.Base* × Multiplier.
 *
 * Rows are weapon-specific (A_Combo1, B_Combo1, A_Heavy_Manual, B_Heavy_Manual, etc.)
 * to allow independent tuning per weapon style.
 *
 * StaminaCostMultiplier: 1.0 for the first hit, 0.0 for subsequent hits.
 * Heavy release montages: StaminaCostMultiplier = 0.0 (cost paid at charge start).
 */
USTRUCT(BlueprintType)
struct REVENANT_API FRVAttackActionMultiplierRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	ERVHitType HitType = ERVHitType::Normal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DamageMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float PoiseDamageMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float StaminaCostMultiplier = 0.f;
};