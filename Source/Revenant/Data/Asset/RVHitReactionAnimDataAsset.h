#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RVHitReactionAnimDataAsset.generated.h"

class UBlendSpace;
class UAnimMontage;

UCLASS()
class REVENANT_API URVHitReactionAnimDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Direction axis: -180 to 180. ABP samples at StaggerDirection during HitReaction state.
	UPROPERTY(EditDefaultsOnly, Category = "Hit")
	TObjectPtr<UBlendSpace> StaggerBlendSpace;

	// Transitions to GetUpMontage on blend-out.
	UPROPERTY(EditDefaultsOnly, Category = "Hit")
	TObjectPtr<UAnimMontage> KnockdownMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Hit")
	TObjectPtr<UAnimMontage> GetUpMontage;

	// Played when HP reaches 0. Leave unassigned for player (death handled via UI/input disable only).
	UPROPERTY(EditDefaultsOnly, Category = "Hit")
	TObjectPtr<UAnimMontage> DeathMontage;

	// 3-stage groggy sequence: Start → Loop (held for GroggyDuration) → End.
	UPROPERTY(EditDefaultsOnly, Category = "Groggy")
	TObjectPtr<UAnimMontage> GroggyStunStartMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Groggy")
	TObjectPtr<UAnimMontage> GroggyStunLoopMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Groggy")
	TObjectPtr<UAnimMontage> GroggyStunEndMontage;
};