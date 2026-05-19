// Source/Revenant/Animation/RVSevarogAnimInstance.h
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
	// --- Locomotion ----------------------------------------------------------

	// Character-local movement direction (-180~180) — both locomotion BS Direction-axis input.
	// Movement speed is controlled directly via CharacterMovementComponent.MaxWalkSpeed.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	float Direction = 0.f;

	// Default walk BS (Sevarog_Locomotion_1D).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	TObjectPtr<UBlendSpace> CachedLocomotionBS;

	// Run BS (Sevarog_TravelMode_Locomotion_1D). Active while bIsRushing == true.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	TObjectPtr<UBlendSpace> CachedRunLocomotionBS;

	// True while ARVSevarogCharacter is closing the gap at rush speed.
	// ABP switches to CachedRunLocomotionBS when this is true.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
	uint8 bIsRushing : 1;

	// --- State ---------------------------------------------------------------

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

	// --- Hit Reaction --------------------------------------------------------

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