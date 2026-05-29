#include "Game/RVGameMode.h"
#include "Controller/RVPlayerController.h"
#include "Kismet/GameplayStatics.h"

void ARVGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	if (ARVPlayerController* PC = Cast<ARVPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PC->RestoreGameInput();
	}
}