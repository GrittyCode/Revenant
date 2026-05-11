#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Interface/RVDamageable.h"
#include "RVAttackActionMultiplierRow.generated.h"

/**
 * Per-hit multipliers stored in DT_AttackStats.csv.
 * Final value = FRVWeaponStatRow.Base* × Multiplier.
 *
 * Multipliers are animation-level and weapon-agnostic —
 * GreatSword_A and GreatSword_B share the same rows (Combo1, Combo2 ...).
 * Only the base stats in FRVWeaponStatRow differ between weapons.
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

	// 1.0 = consume full BaseStaminaCost. 0.0 = free.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float StaminaCostMultiplier = 0.f;
};