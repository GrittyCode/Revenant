#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RVLocomotionAnimDataAsset.generated.h"

class UBlendSpace;


UCLASS(BlueprintType)
class REVENANT_API URVLocomotionAnimDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Default orient-to-movement locomotion (Speed axis).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Locomotion")
	TObjectPtr<UBlendSpace> LocomotionBS;

	// High-speed (run) locomotion (Speed axis).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Locomotion")
	TObjectPtr<UBlendSpace> RunLocomotionBS;

	// Strafe locomotion used during lock-on (Direction + Speed axes).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Locomotion")
	TObjectPtr<UBlendSpace> LockOnLocomotionBS;

	// Guard-state locomotion, free movement (Direction + Speed axes).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Locomotion")
	TObjectPtr<UBlendSpace> GuardLocomotionBS;

	// Guard-state locomotion while locked on (Direction + Speed axes).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Locomotion")
	TObjectPtr<UBlendSpace> GuardLocomotionBS_LockOn;
};