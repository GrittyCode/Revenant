#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/Row/RVPlayerStatRow.h"
#include "RVPlayerDataAsset.generated.h"

UCLASS()
class REVENANT_API URVPlayerDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	FDataTableRowHandle PlayerStatRowHandle;

	const FRVPlayerStatRow* GetPlayerStatRow() const
	{
		if (PlayerStatRowHandle.IsNull()) { return nullptr; }
		return PlayerStatRowHandle.GetRow<FRVPlayerStatRow>(TEXT("URVPlayerDataAsset::GetPlayerStatRow"));
	}
};