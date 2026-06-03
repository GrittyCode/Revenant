#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RVWeaponStatRow.generated.h"

USTRUCT(BlueprintType)
struct REVENANT_API FRVWeaponStatRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BaseDamage = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BasePoiseDamage = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BaseStaminaCost = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AttackRadius = 40.f;

	// Seconds until heavy attack auto-releases at max charge.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.1"))
	float MaxChargeTime = 1.5f;
};
