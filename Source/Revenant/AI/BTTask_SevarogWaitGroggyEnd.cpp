#include "AI/BTTask_SevarogWaitGroggyEnd.h"
#include "AI/RVAIController.h"
#include "Character/Enemy/RVSevarogCharacter.h"

UBTTask_SevarogWaitGroggyEnd::UBTTask_SevarogWaitGroggyEnd()
{
	NodeName    = TEXT("Sevarog Wait Groggy End");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_SevarogWaitGroggyEnd::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return EBTNodeResult::InProgress;
}

void UBTTask_SevarogWaitGroggyEnd::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(Controller)) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	ARVSevarogCharacter* Boss = Controller->GetBossCharacter();
	if (!IsValid(Boss)) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	if (!Boss->IsGroggy())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}