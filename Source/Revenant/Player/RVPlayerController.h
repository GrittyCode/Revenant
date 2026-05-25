#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RVPlayerController.generated.h"

class UInputMappingContext;

DECLARE_LOG_CATEGORY_EXTERN(LogRVPlayerController, Log, All);

UCLASS()
class REVENANT_API ARVPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// Lock all input — called before cutscene playback begins.
	void LockInputForCutscene();

	// Restore game input — called after cutscene ends.
	void UnlockInputAfterCutscene();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Input", Meta = (AllowPrivateAccess = true))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
};