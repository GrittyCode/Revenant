#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RVGameMode.generated.h"

class URVHUDWidget;
class URVBossHPBarWidget;
class URVGameResultWidget;
class ARVSevarogCharacter;
class ARVCharacterBase;

UCLASS()
class REVENANT_API ARVGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	void ShowGameResult(bool bVictory);
	void SetHUDVisible(bool bVisible); 

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "RV|UI")
	TSubclassOf<URVHUDWidget> HUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "RV|UI")
	TSubclassOf<URVBossHPBarWidget> BossHPBarWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "RV|UI")
	TSubclassOf<URVGameResultWidget> GameResultWidgetClass;

	UPROPERTY()
	TObjectPtr<URVHUDWidget> HUDWidget;

	UPROPERTY()
	TObjectPtr<URVBossHPBarWidget> BossHPBarWidget;

	UPROPERTY()
	TObjectPtr<URVGameResultWidget> GameResultWidget;

	TWeakObjectPtr<ARVCharacterBase> PlayerCharRef;

	UFUNCTION()
	void OnPlayerHealthChanged(float NewHealth, float InDelta);

	UFUNCTION()
	void OnPlayerStaminaChanged(float NewStamina, float InDelta);

	UFUNCTION()
	void OnPlayerDeath();

	void OnBossSpawnedHandler(ARVSevarogCharacter* InBoss);

	UFUNCTION()
	void OnBossDefeated();
};