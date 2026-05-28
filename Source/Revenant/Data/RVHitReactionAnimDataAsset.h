#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RVHitReactionAnimDataAsset.generated.h"

class UBlendSpace;
class UAnimMontage;

UCLASS(BlueprintType)
class REVENANT_API URVHitReactionAnimDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Direction axis: -180 to 180. ABP samples at StaggerDirection during HitReaction state.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	TObjectPtr<UBlendSpace> StaggerBlendSpace;

	// Transitions to GetUpMontage on blend-out.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	TObjectPtr<UAnimMontage> KnockdownMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	TObjectPtr<UAnimMontage> GetUpMontage;

	// Played when HP reaches 0. Leave unassigned for player (death handled via UI/input disable only).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	TObjectPtr<UAnimMontage> DeathMontage;

	// 3-stage groggy sequence: Start → Loop (held for GroggyDuration) → End.
	// Leave unassigned for non-boss characters.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Groggy")
	TObjectPtr<UAnimMontage> GroggyStunStartMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Groggy")
	TObjectPtr<UAnimMontage> GroggyStunLoopMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Groggy")
	TObjectPtr<UAnimMontage> GroggyStunEndMontage;
};