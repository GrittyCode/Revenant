#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RVPlayerController.generated.h"

class URVHUDWidget;
class URVBossHPBarWidget;
class URVGameResultWidget;
class ARVCharacterPlayer;
class ARVSevarogCharacter;

DECLARE_LOG_CATEGORY_EXTERN(LogRVPlayerController, Log, All);

UCLASS()
class REVENANT_API ARVPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void RestoreGameInput();
	void OnBossSpawned(ARVSevarogCharacter* InBoss);
	void ShowGameResult(bool bVictory);
	void LockInputForCutscene();
	void UnlockInputAfterCutscene();
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

	//--- Delegate handlers ---------------------------------------------------

	UFUNCTION()
	void OnPlayerHealthChanged(float NewHealthRatio);

	UFUNCTION()
	void OnPlayerStaminaChanged(float NewStaminaRatio);

	UFUNCTION()
	void OnPlayerDeath();

	UFUNCTION()
	void OnBossDefeated();

	TWeakObjectPtr<ARVSevarogCharacter> BossRef;
};