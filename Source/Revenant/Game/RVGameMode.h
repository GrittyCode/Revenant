#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RVGameMode.generated.h"


UCLASS()
class REVENANT_API ARVGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
};