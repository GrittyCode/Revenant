#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "RVBTService_SevarogState.generated.h"

UCLASS()
class REVENANT_API UBTService_RVSevarogState : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_RVSevarogState();

protected:
	virtual void OnSearchStart(FBehaviorTreeSearchData& SearchData) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};