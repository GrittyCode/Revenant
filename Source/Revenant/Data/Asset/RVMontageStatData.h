#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetUserData.h"
#include "Engine/DataTable.h"
#include "RVMontageStatData.generated.h"

struct FRVAttackActionMultiplierRow;

/**
 * Attached to any damage-dealing montage via Details → Asset User Data.
 * FDataTableRowHandle shows a dropdown picker in the editor:
 *   DataTable → DT_AttackStats, RowName → pick from available rows.
 */
UCLASS()
class REVENANT_API URVMontageStatData : public UAssetUserData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "RV|Combat")
	FDataTableRowHandle AttackStatRowHandle;

	const FRVAttackActionMultiplierRow* GetStatRow() const;
};