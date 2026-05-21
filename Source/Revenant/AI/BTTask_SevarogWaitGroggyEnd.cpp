#include "AI/BTTask_SevarogWaitGroggyEnd.h"
#include "AI/RVAIController.h"
#include "Character/Enemy/RVSevarogCharacter.h"

UBTTask_SevarogWaitGroggyEnd::UBTTask_SevarogWaitGroggyEnd()
{
	NodeName            = TEXT("Sevarog Wait Groggy End");
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_SevarogWaitGroggyEnd::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(Controller)) { return EBTNodeResult::Failed; }

	ARVSevarogCharacter* Boss = Controller->GetBossCharacter();
	if (!IsValid(Boss)) { return EBTNodeResult::Failed; }

	// Already recovered before this task ran — succeed immediately.
	if (!Boss->IsGroggy()) { return EBTNodeResult::Succeeded; }

	TWeakObjectPtr<UBehaviorTreeComponent> BTWeak(&OwnerComp);
	TWeakObjectPtr<ARVSevarogCharacter>    BossWeak(Boss);

	Boss->OnBossGroggyEnded.AddWeakLambda(this, [BTWeak, BossWeak, this]()
	{
		if (BossWeak.IsValid()) { BossWeak->OnBossGroggyEnded.RemoveAll(this); }
		if (BTWeak.IsValid())   { FinishLatentTask(*BTWeak.Get(), EBTNodeResult::Succeeded); }
	});

	return EBTNodeResult::InProgress;
}

void UBTTask_SevarogWaitGroggyEnd::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(Controller)) { return; }

	ARVSevarogCharacter* Boss = Controller->GetBossCharacter();
	if (!IsValid(Boss)) { return; }

	// Guard against double-remove (lambda may have already cleaned up on natural completion).
	Boss->OnBossGroggyEnded.RemoveAll(this);
}