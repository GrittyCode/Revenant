#include "AI/BTTask_SevarogRush.h"
#include "AI/RVAIController.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "Data/RVSevarogDataAsset.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_SevarogRush::UBTTask_SevarogRush()
{
	NodeName            = TEXT("Sevarog Rush");
	bNotifyTick         = true;
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_SevarogRush::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FRVRushMemory* Memory = CastInstanceNodeMemory<FRVRushMemory>(NodeMemory);
	Memory->bAttackLaunched = false;

	ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(Controller)) { return EBTNodeResult::Failed; }

	ARVSevarogCharacter* Boss   = Controller->GetBossCharacter();
	APawn*               Player = Controller->GetPlayerPawn();
	if (!IsValid(Boss) || !IsValid(Player)) { return EBTNodeResult::Failed; }

	const float ArrivalRadius = Boss->GetSevarogData()->ArrivalRange;

	Boss->StartRush();

	FAIMoveRequest MoveReq(Player);
	MoveReq.SetAcceptanceRadius(ArrivalRadius * 0.5f);
	MoveReq.SetCanStrafe(false);

	const FPathFollowingRequestResult MoveResult = Controller->MoveTo(MoveReq);

	if (MoveResult.Code == EPathFollowingRequestResult::Failed)
	{
		Boss->EndRush();
		return EBTNodeResult::Failed;
	}

	if (MoveResult.Code == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		// Already in attack range — skip rush movement, go straight to attack.
		Boss->EndRush();
		Boss->RotateToFacePlayer(Player);
		if (!Boss->ExecuteRushAttack()) { return EBTNodeResult::Failed; }

		Memory->bAttackLaunched = true;

		TWeakObjectPtr<UBehaviorTreeComponent> BTWeak(&OwnerComp);
		TWeakObjectPtr<ARVSevarogCharacter>    BossWeak(Boss);
		Boss->OnAttackFinished.AddWeakLambda(this, [BTWeak, BossWeak, this]()
		{
			if (BossWeak.IsValid()) { BossWeak->OnAttackFinished.RemoveAll(this); }
			if (BTWeak.IsValid())   { FinishLatentTask(*BTWeak.Get(), EBTNodeResult::Succeeded); }
		});
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_SevarogRush::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FRVRushMemory* Memory = CastInstanceNodeMemory<FRVRushMemory>(NodeMemory);

	// Attack already launched — OnAttackFinished delegate handles completion.
	if (Memory->bAttackLaunched) { return; }

	ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(Controller)) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	ARVSevarogCharacter* Boss   = Controller->GetBossCharacter();
	APawn*               Player = Controller->GetPlayerPawn();
	if (!IsValid(Boss) || !IsValid(Player)) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	const float Dist          = FVector::Dist(Boss->GetActorLocation(), Player->GetActorLocation());
	const float ArrivalRadius = Boss->GetSevarogData()->ArrivalRange;

	if (Dist <= ArrivalRadius)
	{
		Controller->StopMovement();
		Boss->EndRush();
		Boss->RotateToFacePlayer(Player);

		if (!Boss->ExecuteRushAttack())
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
			return;
		}

		Memory->bAttackLaunched = true;

		TWeakObjectPtr<UBehaviorTreeComponent> BTWeak(&OwnerComp);
		TWeakObjectPtr<ARVSevarogCharacter>    BossWeak(Boss);
		Boss->OnAttackFinished.AddWeakLambda(this, [BTWeak, BossWeak, this]()
		{
			if (BossWeak.IsValid()) { BossWeak->OnAttackFinished.RemoveAll(this); }
			if (BTWeak.IsValid())   { FinishLatentTask(*BTWeak.Get(), EBTNodeResult::Succeeded); }
		});
	}
}

EBTNodeResult::Type UBTTask_SevarogRush::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::AbortTask(OwnerComp, NodeMemory);

	ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
	if (IsValid(Controller))
	{
		Controller->StopMovement();
	}
	return EBTNodeResult::Aborted;
}

void UBTTask_SevarogRush::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(Controller)) { return; }

	ARVSevarogCharacter* Boss = Controller->GetBossCharacter();
	if (!IsValid(Boss)) { return; }

	Boss->OnAttackFinished.RemoveAll(this);

	if (Boss->IsRushing())   { Boss->EndRush(); }
	if (Boss->IsAttacking()) { Boss->ForceEndCurrentAction(); }
}