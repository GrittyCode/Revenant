#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SevarogRush.generated.h"

class ARVSevarogCharacter;

UCLASS()
class REVENANT_API UBTTask_SevarogRush : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SevarogRush();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

private:
	bool bAttackLaunched = false;
	void SubscribeAttackFinished(UBehaviorTreeComponent& OwnerComp, ARVSevarogCharacter* InBoss);
};