#pragma once

#include "CoreMinimal.h"
#include "Data/RVCharacterDataAsset.h"
#include "RVPlayerDataAsset.generated.h"

UCLASS()
class REVENANT_API URVPlayerDataAsset : public URVCharacterDataAsset
{
	GENERATED_BODY()

public:
	//--- Stamina Costs -------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina")
	float DodgeStaminaCost = 30.f;
};