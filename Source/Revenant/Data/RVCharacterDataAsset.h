#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Data/RVCharacterStatRow.h"
#include "RVCharacterDataAsset.generated.h"

UCLASS()
class REVENANT_API URVCharacterDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (DisplayPriority = 0))
	FDataTableRowHandle StatRowHandle;

	FORCEINLINE const FRVCharacterStatRow* GetStatRow() const
	{
		if (StatRowHandle.IsNull()) { return nullptr; }
		return StatRowHandle.GetRow<FRVCharacterStatRow>(TEXT("URVCharacterDataAsset::GetStatRow"));
	}
};