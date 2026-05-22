#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerVolume.h"
#include "RVBossEncounterVolume.generated.h"

class ARVSevarogCharacter;
class USoundBase;

DECLARE_MULTICAST_DELEGATE_OneParam(FRVOnBossSpawned, ARVSevarogCharacter* /*SpawnedBoss*/);

UCLASS()
class REVENANT_API ARVBossEncounterVolume : public ATriggerVolume
{
	GENERATED_BODY()

public:
	ARVBossEncounterVolume();

	// ARVGameMode subscribes via AddUObject in BeginPlay.
	FRVOnBossSpawned OnBossSpawned;

protected:
	virtual void BeginPlay() override;

private:
	//--- Editor-assigned fields ----------------------------------------------

	UPROPERTY(EditInstanceOnly, Category = "RV|Boss")
	TSubclassOf<ARVSevarogCharacter> BossCharacterClass;

	// Place a TargetPoint actor in the level and assign here.
	UPROPERTY(EditInstanceOnly, Category = "RV|Boss")
	TObjectPtr<AActor> BossSpawnPoint;

	UPROPERTY(EditInstanceOnly, Category = "RV|Boss")
	TObjectPtr<USoundBase> BossBGM;

	//--- Fade timing ---------------------------------------------------------

	UPROPERTY(EditInstanceOnly, Category = "RV|Boss|Fade")
	float FadeOutDuration = 0.5f;

	UPROPERTY(EditInstanceOnly, Category = "RV|Boss|Fade")
	float SpawnDelay = 0.3f;

	UPROPERTY(EditInstanceOnly, Category = "RV|Boss|Fade")
	float FadeInDuration = 0.5f;

	UPROPERTY(EditInstanceOnly, Category = "RV|Boss|Fade")
	FLinearColor FadeColor = FLinearColor::Black;

	//--- Runtime state -------------------------------------------------------

	bool bTriggered = false;
	TObjectPtr<ARVSevarogCharacter> SpawnedBoss;

	FTimerHandle SpawnTimerHandle;
	FTimerHandle FadeInTimerHandle;

	//--- Internal flow -------------------------------------------------------

	UFUNCTION()
	void OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor);

	void BeginBossEncounter();
	void OnFadeOutComplete();
	void OnFadeInStart();
};