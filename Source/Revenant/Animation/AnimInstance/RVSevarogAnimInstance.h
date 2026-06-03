#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RVSevarogAnimInstance.generated.h"

class ARVSevarogCharacter;
class UBlendSpace;

UCLASS()
class REVENANT_API URVSevarogAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	//--- Locomotion ----------------------------------------------------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	float Speed = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	TObjectPtr<UBlendSpace> CachedLocomotionBS;

	//--- Hit Reaction --------------------------------------------------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	uint8 bIsInHitReaction : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	float StaggerDirection = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	TObjectPtr<UBlendSpace> CachedStaggerBlendSpace;

private:
	UPROPERTY()
	TObjectPtr<ARVSevarogCharacter> OwnerSevarog;
};