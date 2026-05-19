#include "AI/RVSevarogTasks.h"
#include "AI/RVAIController.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "Data/RVSevarogDataAsset.h"
#include "StateTreeExecutionContext.h"

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------
namespace RVSevarogTasks
{
	static bool CacheRefs(FStateTreeExecutionContext& Context,
		TObjectPtr<ARVAIController>& OutController,
		TObjectPtr<ARVSevarogCharacter>& OutBoss)
	{
		OutController = Cast<ARVAIController>(Context.GetOwner());
		if (!IsValid(OutController)) { return false; }

		OutBoss = OutController->GetBossCharacter();
		return IsValid(OutBoss);
	}

	static float DistToPlayer(const ARVSevarogCharacter* InBoss,
		const ARVAIController* InController)
	{
		APawn* Player = InController->GetPlayerPawn();
		if (!IsValid(Player)) { return 0.f; }

		return FVector::Dist(InBoss->GetActorLocation(), Player->GetActorLocation());
	}
}

// =============================================================================
// FRVChaseTask
// =============================================================================
EStateTreeRunStatus FRVChaseTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& /*Transition*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	if (!RVSevarogTasks::CacheRefs(Context, Data.CachedController, Data.BossCharacter))
	{
		return EStateTreeRunStatus::Failed;
	}

	const URVSevarogDataAsset* SevarogData = Data.BossCharacter->GetSevarogData();
	if (!IsValid(SevarogData)) { return EStateTreeRunStatus::Failed; }

	APawn* Player = Data.CachedController->GetPlayerPawn();
	if (!IsValid(Player)) { return EStateTreeRunStatus::Failed; }

	const float AcceptRadius = SevarogData->AttackRange - 50.f;
	Data.CachedController->MoveToActor(Player, AcceptRadius);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FRVChaseTask::Tick(FStateTreeExecutionContext& Context,
	const float /*DeltaTime*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	const URVSevarogDataAsset* SevarogData = Data.BossCharacter->GetSevarogData();
	if (!IsValid(SevarogData)) { return EStateTreeRunStatus::Failed; }

	const float Dist = RVSevarogTasks::DistToPlayer(Data.BossCharacter, Data.CachedController);

	return Dist <= SevarogData->AttackRange
		? EStateTreeRunStatus::Succeeded
		: EStateTreeRunStatus::Running;
}

// =============================================================================
// FRVRushTask
// =============================================================================
EStateTreeRunStatus FRVRushTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& /*Transition*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	if (!RVSevarogTasks::CacheRefs(Context, Data.CachedController, Data.BossCharacter))
	{
		return EStateTreeRunStatus::Failed;
	}

	Data.bAttackLaunched = false;

	APawn* Player = Data.CachedController->GetPlayerPawn();
	if (!IsValid(Player)) { return EStateTreeRunStatus::Failed; }

	Data.BossCharacter->StartRush();
	Data.CachedController->MoveToActor(Player, 50.f);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FRVRushTask::Tick(FStateTreeExecutionContext& Context,
	const float /*DeltaTime*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	const URVSevarogDataAsset* SevarogData = Data.BossCharacter->GetSevarogData();
	if (!IsValid(SevarogData)) { return EStateTreeRunStatus::Failed; }

	if (!Data.bAttackLaunched)
	{
		const float Dist = RVSevarogTasks::DistToPlayer(Data.BossCharacter, Data.CachedController);
		if (Dist <= SevarogData->RushArrivalRange)
		{
			Data.CachedController->StopMovement();
			Data.BossCharacter->EndRush();
			Data.BossCharacter->ExecuteRushAttack();
			Data.bAttackLaunched = true;
		}
		return EStateTreeRunStatus::Running;
	}

	return Data.BossCharacter->IsAttacking()
		? EStateTreeRunStatus::Running
		: EStateTreeRunStatus::Succeeded;
}

void FRVRushTask::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& /*Transition*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	if (Data.BossCharacter->IsRushing())
	{
		Data.BossCharacter->EndRush();
	}
}

// =============================================================================
// FRVPhaseAttackTask
// =============================================================================
EStateTreeRunStatus FRVPhaseAttackTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& /*Transition*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	TObjectPtr<ARVAIController> Controller = nullptr;
	if (!RVSevarogTasks::CacheRefs(Context, Controller, Data.BossCharacter))
	{
		return EStateTreeRunStatus::Failed;
	}

	Data.BossCharacter->ExecutePhaseAttack();

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FRVPhaseAttackTask::Tick(FStateTreeExecutionContext& Context,
	const float /*DeltaTime*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	return Data.BossCharacter->IsAttacking()
		? EStateTreeRunStatus::Running
		: EStateTreeRunStatus::Succeeded;
}

void FRVPhaseAttackTask::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& /*Transition*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	if (Data.BossCharacter->IsAttacking())
	{
		Data.BossCharacter->ForceEndCurrentAction();
	}
}

// =============================================================================
// FRVSoulSiphonTask
// =============================================================================
EStateTreeRunStatus FRVSoulSiphonTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& /*Transition*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	TObjectPtr<ARVAIController> Controller = nullptr;
	if (!RVSevarogTasks::CacheRefs(Context, Controller, Data.BossCharacter))
	{
		return EStateTreeRunStatus::Failed;
	}

	Data.BossCharacter->ExecuteSoulSiphon();

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FRVSoulSiphonTask::Tick(FStateTreeExecutionContext& Context,
	const float /*DeltaTime*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	return Data.BossCharacter->IsAttacking()
		? EStateTreeRunStatus::Running
		: EStateTreeRunStatus::Succeeded;
}

void FRVSoulSiphonTask::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& /*Transition*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	if (Data.BossCharacter->IsAttacking())
	{
		Data.BossCharacter->ForceEndCurrentAction();
	}
}

// =============================================================================
// FRVSubjugationTask
// =============================================================================
EStateTreeRunStatus FRVSubjugationTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& /*Transition*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	TObjectPtr<ARVAIController> Controller = nullptr;
	if (!RVSevarogTasks::CacheRefs(Context, Controller, Data.BossCharacter))
	{
		return EStateTreeRunStatus::Failed;
	}

	Data.BossCharacter->ExecuteSubjugation();

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FRVSubjugationTask::Tick(FStateTreeExecutionContext& Context,
	const float /*DeltaTime*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	return Data.BossCharacter->IsAttacking()
		? EStateTreeRunStatus::Running
		: EStateTreeRunStatus::Succeeded;
}

void FRVSubjugationTask::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& /*Transition*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	if (Data.BossCharacter->IsAttacking())
	{
		Data.BossCharacter->ForceEndCurrentAction();
	}
}

// =============================================================================
// FRVWaitGroggyEndTask
// =============================================================================
EStateTreeRunStatus FRVWaitGroggyEndTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& /*Transition*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	TObjectPtr<ARVAIController> Controller = nullptr;
	if (!RVSevarogTasks::CacheRefs(Context, Controller, Data.BossCharacter))
	{
		return EStateTreeRunStatus::Failed;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FRVWaitGroggyEndTask::Tick(FStateTreeExecutionContext& Context,
	const float /*DeltaTime*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	return Data.BossCharacter->IsGroggy()
		? EStateTreeRunStatus::Running
		: EStateTreeRunStatus::Succeeded;
}