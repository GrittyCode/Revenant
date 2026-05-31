#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RVDummyAnimInstance.generated.h"

class ARVCharacterBase;

/**
 * Minimal AnimInstance for ARVDummyTarget.
 *   bIsStaggering  — state machine transition to HitReaction state
 *   StaggerDirection — drives BS_Stagger blendspace axis inside HitReaction state
 */
UCLASS()
class REVENANT_API URVDummyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	float Speed = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	float StaggerDirection = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	bool bIsStaggering = false;

private:
	TWeakObjectPtr<ARVCharacterBase> CachedOwner;
};