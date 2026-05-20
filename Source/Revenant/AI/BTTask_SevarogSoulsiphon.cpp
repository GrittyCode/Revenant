#include "AI/BTTask_SevarogSoulsiphon.h"
#include "AI/RVAIController.h"
#include "Character/Enemy/RVSevarogCharacter.h"

UBTTask_SevarogSoulsiphon::UBTTask_SevarogSoulsiphon()
{
	NodeName            = TEXT("Sevarog Soul Siphon");
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_SevarogSoulsiphon::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(Controller)) { return EBTNodeResult::Failed; }

	ARVSevarogCharacter* Boss = Controller->GetBossCharacter();
	if (!IsValid(Boss)) { return EBTNodeResult::Failed; }

	Boss->RotateToFacePlayer();
	if (!Boss->ExecuteSoulSiphon()) { return EBTNodeResult::Failed; }

	TWeakObjectPtr<UBehaviorTreeComponent> BTCompWeak(&OwnerComp);
	TWeakObjectPtr<ARVSevarogCharacter>    BossWeak(Boss);

	Boss->OnAttackFinished.AddWeakLambda(this, [BTCompWeak, BossWeak, this]()
	{
		if (BossWeak.IsValid()) { BossWeak->OnAttackFinished.RemoveAll(this); }
		if (BTCompWeak.IsValid()) { FinishLatentTask(*BTCompWeak.Get(), EBTNodeResult::Succeeded); }
	});

	return EBTNodeResult::InProgress;
}

void UBTTask_SevarogSoulsiphon::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(Controller)) { return; }

	ARVSevarogCharacter* Boss = Controller->GetBossCharacter();
	if (!IsValid(Boss)) { return; }

	Boss->OnAttackFinished.RemoveAll(this);

	if (Boss->IsAttacking())
	{
		Boss->ForceEndCurrentAction();
	}
}