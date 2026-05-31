#include "Game/RVGameMode.h"
#include "Controller/RVPlayerController.h"


void ARVGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	if (ARVPlayerController* PC = Cast<ARVPlayerController>(NewPlayer))
	{
		PC->RestoreGameInput();
	}
}