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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Hit")
	TObjectPtr<UBlendSpace> StaggerBlendSpace;

	// Transitions to GetUpMontage on blend-out.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Hit")
	TObjectPtr<UAnimMontage> KnockdownMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Hit")
	TObjectPtr<UAnimMontage> GetUpMontage;

	// Boss-only groggy — 3-stage: Start → Loop (driven by GroggyDuration timer) → End
	UPROPERTY(EditDefaultsOnly, Category = "RV|Hit|Groggy")
	TObjectPtr<UAnimMontage> GroggyStunStartMontage;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Hit|Groggy")
	TObjectPtr<UAnimMontage> GroggyStunLoopMontage;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Hit|Groggy")
	TObjectPtr<UAnimMontage> GroggyStunEndMontage;
};