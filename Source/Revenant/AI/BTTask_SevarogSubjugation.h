#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SevarogSubjugation.generated.h"

UCLASS()
class REVENANT_API UBTTask_SevarogSubjugation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SevarogSubjugation();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
};