#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RVPlayerController.generated.h"

class UInputMappingContext;
class URVHUDWidget;
class URVBossHPBarWidget;
class URVGameResultWidget;
class ARVCharacterBase;
class ARVSevarogCharacter;

DECLARE_LOG_CATEGORY_EXTERN(LogRVPlayerController, Log, All);

UCLASS()
class REVENANT_API ARVPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// Called by ARVGameMode::BeginPlay after level load.
	// Locks input and hides cursor — correct state after every OpenLevel.
	void RestoreGameInput();

	// Called by ARVBossEncounterVolume via OnBossSpawned delegate.
	void OnBossSpawned(ARVSevarogCharacter* InBoss);

	// Called by ARVGameMode when the game ends (victory or defeat).
	void ShowGameResult(bool bVictory);

	// Called by ARVBossEncounterVolume / cutscene system.
	void LockInputForCutscene();
	void UnlockInputAfterCutscene();

	// Called by ARVGameMode to toggle HUD visibility during cutscene.
	void SetHUDVisible(bool bVisible);

protected:
	virtual void BeginPlay() override;

private:
	//--- Widget classes (assign in BP_PlayerController) ----------------------

	UPROPERTY(EditDefaultsOnly, Category = "RV|UI")
	TSubclassOf<URVHUDWidget> HUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "RV|UI")
	TSubclassOf<URVBossHPBarWidget> BossHPBarWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "RV|UI")
	TSubclassOf<URVGameResultWidget> GameResultWidgetClass;

	//--- Runtime widget instances --------------------------------------------

	UPROPERTY()
	TObjectPtr<URVHUDWidget> HUDWidget;

	UPROPERTY()
	TObjectPtr<URVBossHPBarWidget> BossHPBarWidget;

	UPROPERTY()
	TObjectPtr<URVGameResultWidget> GameResultWidget;

	//--- Player attribute delegate handlers ----------------------------------

	TWeakObjectPtr<ARVCharacterBase> PlayerCharRef;

	UFUNCTION()
	void OnPlayerHealthChanged(float NewHealth, float InDelta);

	UFUNCTION()
	void OnPlayerStaminaChanged(float NewStamina, float InDelta);

	UFUNCTION()
	void OnPlayerDeath();

	//--- Boss delegate handlers ----------------------------------------------

	UFUNCTION()
	void OnBossDefeated();

	TWeakObjectPtr<ARVSevarogCharacter> BossRef;
	
};