#include "AI/BTTask_SevarogAttackBase.h"
#include "AI/RVAIController.h"
#include "Character/Enemy/RVSevarogCharacter.h"

UBTTask_SevarogAttackBase::UBTTask_SevarogAttackBase()
{
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_SevarogAttackBase::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(Controller)) { return EBTNodeResult::Failed; }

	ARVSevarogCharacter* Boss = Controller->GetBossCharacter();
	if (!IsValid(Boss)) { return EBTNodeResult::Failed; }

	if (!LaunchAttack(Boss)) { return EBTNodeResult::Failed; }

	TWeakObjectPtr<UBehaviorTreeComponent> BTWeak(&OwnerComp);
	TWeakObjectPtr<ARVSevarogCharacter>    BossWeak(Boss);

	Boss->OnAttackFinished.AddWeakLambda(this, [BTWeak, BossWeak, this]()
	{
		if (BossWeak.IsValid()) { BossWeak->OnAttackFinished.RemoveAll(this); }
		if (BTWeak.IsValid())   { FinishLatentTask(*BTWeak.Get(), EBTNodeResult::Succeeded); }
	});

	return EBTNodeResult::InProgress;
}

void UBTTask_SevarogAttackBase::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(Controller)) { return; }

	ARVSevarogCharacter* Boss = Controller->GetBossCharacter();
	if (!IsValid(Boss)) { return; }

	Boss->OnAttackFinished.RemoveAll(this);

	if (Boss->IsAttacking()) { Boss->ForceEndCurrentAction(); }
}