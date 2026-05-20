#include "AI/RVAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BrainComponent.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "Kismet/GameplayStatics.h"

ARVAIController::ARVAIController()
{
}

void ARVAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ensureMsgf(IsValid(Cast<ARVSevarogCharacter>(InPawn)),
		TEXT("[%s] ARVAIController possessed a non-Sevarog pawn"), *GetNameSafe(this));

	if (IsValid(BehaviorTreeAsset))
	{
		// RunBehaviorTree calls UseBlackboard internally with the BT asset's embedded blackboard.
		RunBehaviorTree(BehaviorTreeAsset);
	}
}

void ARVAIController::OnUnPossess()
{
	Super::OnUnPossess();

	if (UBrainComponent* Brain = GetBrainComponent())
	{
		Brain->StopLogic(TEXT("Unpossessed"));
	}
	StopMovement();
}

ARVSevarogCharacter* ARVAIController::GetBossCharacter() const
{
	return Cast<ARVSevarogCharacter>(GetPawn());
}

APawn* ARVAIController::GetPlayerPawn() const
{
	return UGameplayStatics::GetPlayerPawn(this, 0);
}

void ARVAIController::SetFocusToPlayer()
{
	APawn* Player = GetPlayerPawn();
	if (!IsValid(Player)) { return; }
	SetFocus(Player);
}

void ARVAIController::ClearBossFocus()
{
	ClearFocus(EAIFocusPriority::Gameplay);
}