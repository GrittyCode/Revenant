#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SevarogAttackBase.generated.h"

class ARVSevarogCharacter;

UCLASS(Abstract)
class REVENANT_API UBTTask_SevarogAttackBase : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SevarogAttackBase();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

	/** Returns false if the boss cannot execute the action (groggy, already attacking, unassigned montage). */
	virtual bool LaunchAttack(ARVSevarogCharacter* InBoss) PURE_VIRTUAL(UBTTask_SevarogAttackBase::LaunchAttack, return false;);

	// Binds before LaunchAttack to avoid a race where a short montage ends before binding completes.
	void SubscribeAttackFinished(UBehaviorTreeComponent& OwnerComp, ARVSevarogCharacter* InBoss);
};