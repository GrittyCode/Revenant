#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SevarogSoulsiphon.generated.h"

UCLASS()
class REVENANT_API UBTTask_SevarogSoulsiphon : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SevarogSoulsiphon();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
};