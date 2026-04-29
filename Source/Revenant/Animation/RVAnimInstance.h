// Source/Revenant/Animation/RVAnimInstance.h
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RVAnimInstance.generated.h"

class URVEquipmentComponent;
class URVComboComponent;
class URVWeaponDataAsset;
class UBlendSpace;

UCLASS()
class REVENANT_API URVAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUninitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	// --- Locomotion -----------------------------------------------------------

	// XY plane speed only — vertical velocity excluded to avoid blending artifacts during jumps.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	float Speed;

	// -180 ~ 180 degrees, 0 = forward, 90 = right, -90 = left, -180 = back
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	float Direction;

	// Updated via OnWeaponChanged delegate — fed into ABP Blend Space Evaluator.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	TObjectPtr<UBlendSpace> CachedLocomotionBS;

	// --- State ----------------------------------------------------------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	uint8 bIsInAir : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	uint8 bIsAttacking : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	uint8 bIsLockedOn : 1;

private:
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<URVEquipmentComponent> EquipmentComponent;

	UPROPERTY()
	TObjectPtr<URVComboComponent> ComboComponent;

	// Bound to URVEquipmentComponent::OnWeaponChanged
	UFUNCTION()
	void OnWeaponChangedHandler(URVWeaponDataAsset* NewWeaponData);
};