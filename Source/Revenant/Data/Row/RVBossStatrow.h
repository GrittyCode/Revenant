#pragma once

#include "CoreMinimal.h"
#include "Data/Row/RVCharacterStatRow.h"
#include "RVBossStatRow.generated.h"

USTRUCT(BlueprintType)
struct REVENANT_API FRVBossStatRow : public FRVCharacterStatRow
{
	GENERATED_BODY()

	// Final damage = BaseDamage x DT_AttackStats.DamageMultiplier (via URVMontageStatData).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float BaseDamage = 80.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float BasePoiseDamage = 40.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float AttackRadius = 55.f;

	// Duration the boss remains in Groggy stun (Start → Loop held for this duration → End).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float GroggyDuration = 4.f;
};