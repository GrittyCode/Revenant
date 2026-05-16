#include "AI/RVAIController.h"
#include "Character/Enemy/RVBossCharacter.h"
#include "Components/StateTreeAIComponent.h"
#include "Kismet/GameplayStatics.h"

ARVAIController::ARVAIController()
{
	StateTreeAIComponent = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAIComponent"));
}

void ARVAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ensureMsgf(IsValid(Cast<ARVBossCharacter>(InPawn)),
		TEXT("[%s] ARVAIController possessed a non-boss pawn"), *GetNameSafe(this));

	StateTreeAIComponent->StartLogic();
}

void ARVAIController::OnUnPossess()
{
	StateTreeAIComponent->StopLogic(TEXT("UnPossess"));
	Super::OnUnPossess();
}

ARVBossCharacter* ARVAIController::GetBossCharacter() const
{
	return Cast<ARVBossCharacter>(GetPawn());
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