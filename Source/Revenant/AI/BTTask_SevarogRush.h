#pragma once

#include "CoreMinimal.h"
#include "AI/BTTask_SevarogAttackBase.h"
#include "BTTask_SevarogRush.generated.h"

class ARVSevarogCharacter;

UCLASS()
class REVENANT_API UBTTask_SevarogRush : public UBTTask_SevarogAttackBase
{
	GENERATED_BODY()

public:
	UBTTask_SevarogRush();

protected:
	virtual bool LaunchAttack(ARVSevarogCharacter* InBoss) override;

	virtual EBTNodeResult::Type ExecuteTask   (UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask     (UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void                TickTask      (UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void                OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
};