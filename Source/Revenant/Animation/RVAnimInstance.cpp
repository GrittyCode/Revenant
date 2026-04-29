// Source/Revenant/Animation/RVAnimInstance.cpp
#include "Animation/RVAnimInstance.h"
#include "Component/RVComboComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Data/RVWeaponDataAsset.h"
#include "KismetAnimationLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void URVAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerCharacter = Cast<ACharacter>(GetOwningActor());
	if (!IsValid(OwnerCharacter)) { return; }

	EquipmentComponent = OwnerCharacter->FindComponentByClass<URVEquipmentComponent>();
	ComboComponent     = OwnerCharacter->FindComponentByClass<URVComboComponent>();

	if (IsValid(EquipmentComponent))
	{
		EquipmentComponent->OnWeaponChanged.AddDynamic(
			this, &URVAnimInstance::OnWeaponChangedHandler);

		// Read initial value in case EquipmentComponent::BeginPlay already ran
		OnWeaponChangedHandler(EquipmentComponent->GetCurrentWeaponData());
	}
}

void URVAnimInstance::NativeUninitializeAnimation()
{
	if (IsValid(EquipmentComponent))
	{
		EquipmentComponent->OnWeaponChanged.RemoveDynamic(
			this, &URVAnimInstance::OnWeaponChangedHandler);
	}

	Super::NativeUninitializeAnimation();
}

void URVAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!IsValid(OwnerCharacter)) { return; }

	// --- Locomotion -----------------------------------------------------------

	Speed = OwnerCharacter->GetVelocity().Size2D();

	Direction = UKismetAnimationLibrary::CalculateDirection(
		OwnerCharacter->GetVelocity(),
		OwnerCharacter->GetActorRotation());

	// --- State ----------------------------------------------------------------

	bIsInAir     = OwnerCharacter->GetCharacterMovement()->IsFalling();
	bIsAttacking = IsValid(ComboComponent) ? ComboComponent->IsComboActive() : false;

	// TODO: Wire URVCombatComponent::IsLockedOn() in Phase 4
	bIsLockedOn = false;
}

void URVAnimInstance::OnWeaponChangedHandler(URVWeaponDataAsset* NewWeaponData)
{
	CachedLocomotionBS = IsValid(NewWeaponData) ? NewWeaponData->LocomotionBS : nullptr;
}