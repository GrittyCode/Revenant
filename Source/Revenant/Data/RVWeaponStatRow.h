#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RVWeaponStatRow.generated.h"

USTRUCT(BlueprintType)
struct REVENANT_API FRVWeaponStatRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BasePoiseDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BaseStaminaCost;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AttackRadius = 40.f;
};