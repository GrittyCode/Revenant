#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RVSevarogAnimInstance.generated.h"

class ARVSevarogCharacter;
class URVCombatStateComponent;
class URVHitReactionComponent;
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

	// Movement speed — 1D BlendSpace (LocomotionBS) X-axis input.
	// CharacterMovement.MaxWalkSpeed controls normal vs. rush automatically.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	float Speed = 0.f;

	// Single 1D BlendSpace covering Idle → Walk → Rush (Speed axis).
	// Assigned from URVSevarogDataAsset::LocomotionBS at init.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	TObjectPtr<UBlendSpace> CachedLocomotionBS;

	//--- State ---------------------------------------------------------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	uint8 bIsInAir : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	uint8 bIsAttacking : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	uint8 bIsInHitReaction : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	uint8 bIsKnockedDown : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	uint8 bIsGroggy : 1;

	//--- Hit Reaction --------------------------------------------------------

	// Character-local hit angle (-180~180) — Stagger BS Direction-axis input.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	float StaggerDirection = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	TObjectPtr<UBlendSpace> CachedStaggerBlendSpace;

private:
	UPROPERTY()
	TObjectPtr<ARVSevarogCharacter> OwnerSevarog;

	UPROPERTY()
	TObjectPtr<URVCombatStateComponent> CombatStateComponent;

	UPROPERTY()
	TObjectPtr<URVHitReactionComponent> HitReactionComponent;
};