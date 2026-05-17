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

	UPROPERTY(EditDefaultsOnly, Category = "RV|Hit")
	TObjectPtr<UAnimMontage> GroggyStartMontage;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Hit")
	TObjectPtr<UAnimMontage> GroggyEndMontage;
};
