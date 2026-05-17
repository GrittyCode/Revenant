#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RVEnemyStatRow.generated.h"


USTRUCT(BlueprintType)
struct REVENANT_API FRVEnemyStatRow : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	//--- Attribute -----------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxHP = 3000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxPoise = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float StaggerDuration = 0.5f;

	//--- Movement ------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MoveSpeed = 300.f;

	//--- Attack --------------------------------------------------------------
	// Final damage = BaseDamage × DT_AttackStats.DamageMultiplier (via URVMontageStatData)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BaseDamage = 80.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BasePoiseDamage = 40.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AttackRadius = 55.f;
};