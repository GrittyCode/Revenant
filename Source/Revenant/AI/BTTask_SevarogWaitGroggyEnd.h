#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SevarogWaitGroggyEnd.generated.h"

UCLASS()
class REVENANT_API UBTTask_SevarogWaitGroggyEnd : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SevarogWaitGroggyEnd();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
};