#include "Game/RVGameMode.h"
#include "Player/RVPlayerController.h"
#include "Kismet/GameplayStatics.h"

void ARVGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Restore game-only input and hide cursor after every level load (including Retry).
	// ShowGameResult() in ARVPlayerController switches to UIOnly + cursor visible;
	// OpenLevel triggers a fresh BeginPlay, so this is the correct site to undo that state.
	if (ARVPlayerController* PC = Cast<ARVPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PC->RestoreGameInput();
	}
}