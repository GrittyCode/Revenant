#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RVPlayerController.generated.h"

class URVHUDWidget;
class URVBossHPBarWidget;
class URVGameResultWidget;
class ARVCharacterPlayer;
class ARVSevarogCharacter;
class UInputMappingContext;
class UInputAction;
class UEnhancedInputLocalPlayerSubsystem;

DECLARE_LOG_CATEGORY_EXTERN(LogRVPlayerController, Log, All);

/** Broadcast by ARVPlayerController when the player presses IA_SkipCutscene.
 *  ARVBossEncounterVolume subscribes during StartCutscene() and stops the LevelSequence. */
DECLARE_MULTICAST_DELEGATE(FRVOnCutsceneSkipRequested);

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

	/** ARVBossEncounterVolume subscribes to this delegate in StartCutscene()
	 *  and unsubscribes in OnCutsceneFinished() to avoid dangling handles. */
	FRVOnCutsceneSkipRequested OnCutsceneSkipRequested;

protected:
	virtual void BeginPlay() override;

	/** Binds IA_SkipCutscene permanently.
	 *  The action only fires when IMC_Cutscene is active — no effect during gameplay. */
	virtual void SetupInputComponent() override;

private:
	//--- Input ---------------------------------------------------------------

	void OnSkipCutsceneInput();

	UEnhancedInputLocalPlayerSubsystem* GetInputSubsystem() const;

	/** Assign IMC_Cutscene asset in BP_RVPlayerController.
	 *  Contains only IA_SkipCutscene; replaces IMC_Player during cutscenes. */
	UPROPERTY(EditDefaultsOnly, Category = "RV|Input|Cutscene")
	TObjectPtr<UInputMappingContext> CutsceneMappingContext;

	/** Assign IA_SkipCutscene asset in BP_RVPlayerController. */
	UPROPERTY(EditDefaultsOnly, Category = "RV|Input|Cutscene")
	TObjectPtr<UInputAction> SkipCutsceneAction;

	/** Cached during LockInputForCutscene(); restored and cleared in UnlockInputAfterCutscene(). */
	TWeakObjectPtr<UInputMappingContext> CachedPlayerMappingContext;

	//--- Widget classes (assign in BP_RVPlayerController) --------------------

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

	void OnPlayerHealthChanged(float NewHealthRatio);
	void OnPlayerStaminaChanged(float NewStaminaRatio);
	void OnPlayerDeath();
	void OnBossDefeated();

	TWeakObjectPtr<ARVSevarogCharacter> BossRef;
};