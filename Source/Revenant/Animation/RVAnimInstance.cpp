// Fill out your copyright notice in the Description page of Project Settings.


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

	// Speed : XY Plane only - vertical velocity exclude
	Speed = OwnerCharacter->GetVelocity().Size();


	// Direction : Angle between actor forward and velocity vector ( -180 ~ 180 )
	// UKismetAnimationLibrary::CalculateDirection handles the math
	Direction = UKismetAnimationLibrary::CalculateDirection(
		OwnerCharacter->GetVelocity(),
		OwnerCharacter->GetActorRotation());
	
	
	bIsInAir = Movement->IsFalling();
	
	// Read attack state from ComboComponent
	// TODO: replace with URVCombatComponent::IsAttacking() once CombatComponent is implemented (covers heavy attack, skill attack, etc.)
	URVComboComponent* ComboComponent = OwnerCharacter->FindComponentByClass<URVComboComponent>();
	bIsAttacking = IsValid(ComboComponent) ? ComboComponent->IsComboActive() : false;
}
