#include "Player/RVPlayerController.h"
#include "Camera/PlayerCameraManager.h"

DEFINE_LOG_CATEGORY(LogRVPlayerController);

void ARVPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void ARVPlayerController::LockInputForCutscene()
{
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);
	SetInputMode(FInputModeUIOnly());
}

void ARVPlayerController::UnlockInputAfterCutscene()
{
	FlushPressedKeys();
	ResetIgnoreMoveInput();
	ResetIgnoreLookInput();
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
}