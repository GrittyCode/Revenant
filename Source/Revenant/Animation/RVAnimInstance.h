// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RVAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class REVENANT_API URVAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
protected:
	// Locomotion Parameters = fed into BS_Locomotion
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	float Speed;
	
	// -180 ~ 180 degrees, 0 = forward, 90 = right, -90 = left  -180 = back
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	float Direction;
	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	uint8 bIsInAir : 1;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	uint8 bIsAttacking : 1;
	
	
private:
	TObjectPtr<ACharacter> OwnerCharacter;
};
