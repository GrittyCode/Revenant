#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerVolume.h"
#include "RVBossEncounterVolume.generated.h"

class ARVSevarogCharacter;
class ALevelSequenceActor;
class USoundBase;
class UAudioComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FRVOnBossSpawned, ARVSevarogCharacter* /*SpawnedBoss*/);

UCLASS()
class REVENANT_API ARVBossEncounterVolume : public ATriggerVolume
{
	GENERATED_BODY()

public:
	ARVBossEncounterVolume();

	FRVOnBossSpawned OnBossSpawned;

protected:
	virtual void BeginPlay() override;

private:
	//--- Editor-assigned fields ----------------------------------------------

	UPROPERTY(EditInstanceOnly, Category = "RV|Boss")
	TSubclassOf<ARVSevarogCharacter> BossCharacterClass;

	UPROPERTY(EditInstanceOnly, Category = "RV|Boss")
	TObjectPtr<AActor> BossSpawnPoint;

	UPROPERTY(EditInstanceOnly, Category = "RV|Boss|Audio")
	TObjectPtr<USoundBase> CutsceneBGM;
	
	UPROPERTY()
	TObjectPtr<UAudioComponent> CutsceneBGMAudioComponent;

	UPROPERTY(EditInstanceOnly, Category = "RV|Boss|Audio")
	TObjectPtr<USoundBase> CombatBGM;

	UPROPERTY(EditInstanceOnly, Category = "RV|Boss|Cutscene")
	TObjectPtr<ALevelSequenceActor> CutsceneSequenceActor;

	UPROPERTY(EditInstanceOnly, Category = "RV|Boss|Cutscene")
	TObjectPtr<UAnimMontage> BossIntroMontage;
	

	//--- Runtime state -------------------------------------------------------

	bool bTriggered = false;
	TObjectPtr<ARVSevarogCharacter> SpawnedBoss;
	
	//--- Internal flow -------------------------------------------------------

	UFUNCTION()
	void OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor);

	void BeginBossEncounter();

	void StartCutscene();

	void PauseBossAI();
	void ResumeBossAI();

	UFUNCTION()
	void OnCutsceneFinished();
};