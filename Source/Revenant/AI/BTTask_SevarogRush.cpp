#include "AI/BTTask_SevarogRush.h"
#include "AI/RVAIController.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "Data/RVSevarogDataAsset.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_SevarogRush::UBTTask_SevarogRush()
{
	NodeName = TEXT("Sevarog Rush");
	bNotifyTick = true;
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_SevarogRush::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FRVRushMemory* Memory = CastInstanceNodeMemory<FRVRushMemory>(NodeMemory);
	Memory->bAttackLaunched = false;

	ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(Controller)) { return EBTNodeResult::Failed; }

	ARVSevarogCharacter* Boss = Controller->GetBossCharacter();
	APawn* Player = Controller->GetPlayerPawn();
	if (!IsValid(Boss) || !IsValid(Player)) { return EBTNodeResult::Failed; }

	Boss->StartRush();
	Controller->MoveToActor(Player, Boss->GetSevarogData()->AttackRadius * 0.5f);
	return EBTNodeResult::InProgress;
}

void UBTTask_SevarogRush::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FRVRushMemory* Memory = CastInstanceNodeMemory<FRVRushMemory>(NodeMemory);

	ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(Controller))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	ARVSevarogCharacter* Boss = Controller->GetBossCharacter();
	APawn* Player = Controller->GetPlayerPawn();
	if (!IsValid(Boss) || !IsValid(Player))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (!Memory->bAttackLaunched)
	{
		const float Dist = FVector::Dist(Boss->GetActorLocation(), Player->GetActorLocation());
		const float ArrivalRadius = Boss->GetSevarogData()->AttackRadius * 0.5f;

		if (Dist <= ArrivalRadius)
		{
			Controller->StopMovement();
			Boss->EndRush();
			Boss->RotateToFacePlayer();

			if (!Boss->ExecuteRushAttack())
			{
				FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
				return;
			}
			Memory->bAttackLaunched = true;
		}
		else
		{
			UPathFollowingComponent* PFC = Controller->GetPathFollowingComponent();
			if (IsValid(PFC) && PFC->GetStatus() != EPathFollowingStatus::Moving)
			{
				Controller->MoveToActor(Player, ArrivalRadius * 0.5f);
			}
		}
	}
	else if (!Boss->IsAttacking())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
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

	if (Boss->IsRushing())
	{
		Boss->EndRush();
	}

	if (Boss->IsAttacking())
	{
		Boss->ForceEndCurrentAction();
	}
}
