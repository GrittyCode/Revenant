#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/RVBossCharacter.h"
#include "RVSevarogCharacter.generated.h"

class URVSevarogDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRVOnSoulSiphonCompleted, bool, bHealed);

UCLASS()
class REVENANT_API ARVSevarogCharacter : public ARVBossCharacter
{
	GENERATED_BODY()

public:
	// --- StateTree task interface --------------------------------------------

	// Channel Soul_Siphon — heals if not interrupted.
	// StateTree task binds OnSoulSiphonCompleted to detect completion.
	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	void ExecuteSoulSiphon();

	// Summon ground field via AnimNotify_SpawnGroundField mid-montage.
	// StateTree task polls IsAttacking() == false to detect completion.
	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	void ExecuteSubjugation();

	// Fired when the Soul_Siphon montage finishes (interrupted or not).
	// bHealed == true only when the montage completed uninterrupted.
	UPROPERTY(BlueprintAssignable, Category = "RV|Boss")
	FRVOnSoulSiphonCompleted OnSoulSiphonCompleted;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss")
	TObjectPtr<URVSevarogDataAsset> SevarogData;

private:
	// Called by AnimNotify_SpawnGroundField at the key frame of SubjugationMontage.
	void SpawnGroundField();
};