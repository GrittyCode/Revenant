#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RVGameMode.generated.h"

// ARVGameMode is responsible for game-rule lifecycle only.
// All UI creation, widget management, and delegate binding is handled by ARVPlayerController.
UCLASS()
class REVENANT_API ARVGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
};