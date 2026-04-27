#include "Animation/RVAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "Component/RVComboComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void URVAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerCharacter = Cast<ACharacter>(GetOwningActor());
}

void URVAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!IsValid(OwnerCharacter))
	{
		return;
	}

	UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();

	// XY plane speed only — vertical velocity excluded to avoid blending artifacts during jumps.
	// Size2D() discards Z; Size() (3D) was previously used here, which was incorrect.
	Speed = OwnerCharacter->GetVelocity().Size2D();

	// Angle between actor forward and velocity vector (-180 ~ 180).
	// UKismetAnimationLibrary::CalculateDirection handles the signed angle math.
	Direction = UKismetAnimationLibrary::CalculateDirection(
		OwnerCharacter->GetVelocity(),
		OwnerCharacter->GetActorRotation());

	bIsInAir = Movement->IsFalling();

	// Read attack state from ComboComponent.
	// TODO(): replace with URVCombatComponent::IsAttacking() once CombatComponent is implemented
	//              (covers heavy attack, skill attack, etc.)
	
	URVComboComponent* ComboComponent = OwnerCharacter->FindComponentByClass<URVComboComponent>();
	bIsAttacking = IsValid(ComboComponent) ? ComboComponent->IsComboActive() : false;
}