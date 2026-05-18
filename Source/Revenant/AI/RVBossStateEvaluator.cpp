#include "AI/RVBossStateEvaluator.h"
#include "AI/RVAIController.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "StateTreeExecutionContext.h"

void FRVBossStateEvaluator::TreeStart(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	// UStateTreeAIComponent passes *GetOwner() (the AIController) as context owner.
	Data.CachedAIController = Cast<ARVAIController>(Context.GetOwner());
	
	ensureMsgf(IsValid(Data.CachedAIController),
		TEXT("[FRVBossStateEvaluator] Context owner is not ARVAIController"));
	
	if (!IsValid(Data.CachedAIController)) { return; }

	Data.BossCharacter = Cast<ARVSevarogCharacter>(Data.CachedAIController->GetPawn());
	ensureMsgf(IsValid(Data.BossCharacter),
		TEXT("[FRVBossStateEvaluator] Pawn is not ARVSevarogCharacter"));
}

void FRVBossStateEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	if (!IsValid(Data.BossCharacter) || !IsValid(Data.CachedAIController)) { return; }

	Data.bIsGroggy       = Data.BossCharacter->IsGroggy();
	Data.bIsAttacking    = Data.BossCharacter->IsAttacking();
	Data.CurrentPhase    = Data.BossCharacter->GetCurrentPhase();
	Data.BossHealthRatio = Data.BossCharacter->GetHealthRatio();

	Data.PlayerPawn = Data.CachedAIController->GetPlayerPawn();

	Data.DistToPlayer = IsValid(Data.PlayerPawn)
		? FVector::Dist(
			Data.BossCharacter->GetActorLocation(),
			Data.PlayerPawn->GetActorLocation())
		: 0.f;
}