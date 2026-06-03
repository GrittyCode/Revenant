#include "Animation/AnimInstance/RVPlayerAnimInstance.h"
#include "Data/Asset/RVWeaponDataAsset.h"
#include "Data/Asset/RVLocomotionAnimDataAsset.h"
#include "Data/Asset/RVHitReactionAnimDataAsset.h"
#include "Component/Combat/RVCombatStateComponent.h"
#include "Character/Player/RVCharacterPlayer.h"
#include "KismetAnimationLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

void URVPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerCharacter = Cast<ARVCharacterPlayer>(GetOwningActor());
	ensureMsgf(IsValid(OwnerCharacter),
	           TEXT("[URVPlayerAnimInstance] Owner is not ARVCharacterPlayer — check ABP assignment"));
	if (!IsValid(OwnerCharacter)) { return; }

	MaxLocomotionSpeed = OwnerCharacter->GetSprintSpeed();

	OwnerCharacter->GetOnWeaponChanged().AddUObject(this, &URVPlayerAnimInstance::OnWeaponChangedHandler);

	OnWeaponChangedHandler(OwnerCharacter->GetEquipmentComponent()->GetCurrentWeaponData());
}

void URVPlayerAnimInstance::NativeUninitializeAnimation()
{
	if (IsValid(OwnerCharacter))
	{
		OwnerCharacter->GetOnWeaponChanged().RemoveAll(this);
	}

	Super::NativeUninitializeAnimation();
}

void URVPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!IsValid(OwnerCharacter)) { return; }

	const UWorld* W = GetWorld();
	if (!W || !W->IsGameWorld()) { return; }

	Speed = OwnerCharacter->GetVelocity().Size2D();
	NormalizedSpeed = FMath::Clamp(Speed / MaxLocomotionSpeed, 0.f, 1.f);
	Direction = UKismetAnimationLibrary::CalculateDirection(
		OwnerCharacter->GetVelocity(),
		OwnerCharacter->GetActorRotation());

	bIsInAir = OwnerCharacter->GetCharacterMovement()->IsFalling();
	bIsGuarding = OwnerCharacter->HasCombatState(ERVCombatState::Guarding);
	bIsLockedOn = OwnerCharacter->IsLockedOn();
	bIsSprinting = OwnerCharacter->IsSprinting();
	bIsAttacking = OwnerCharacter->HasCombatState(ERVCombatState::Attacking);
	bIsKnockedDown = OwnerCharacter->HasCombatState(ERVCombatState::Knockdown);
	bIsInHitReaction = OwnerCharacter->HasCombatState(ERVCombatState::HitReaction);
	StaggerDirection = OwnerCharacter->GetStaggerDirection();
}

void URVPlayerAnimInstance::OnWeaponChangedHandler(URVWeaponDataAsset* NewWeaponData)
{
	if (!IsValid(NewWeaponData))
	{
		CachedLocomotionBS = nullptr;
		CachedRunLocomotionBS = nullptr;
		CachedLockOnLocomotionBS = nullptr;
		CachedGuardLocomotionBS = nullptr;
		CachedGuardLocomotionBS_LockOn = nullptr;
		CachedStaggerBlendSpace = nullptr;
		return;
	}

	if (!ensureMsgf(IsValid(NewWeaponData->LocomotionAnimData),
	                TEXT("[URVPlayerAnimInstance] WeaponData '%s' has no LocomotionAnimData"),
	                *GetNameSafe(NewWeaponData))) { return; }

	if (!ensureMsgf(IsValid(NewWeaponData->HitReactionAnimData),
	                TEXT("[URVPlayerAnimInstance] WeaponData '%s' has no HitReactionAnimData"),
	                *GetNameSafe(NewWeaponData))) { return; }

	CachedLocomotionBS = NewWeaponData->LocomotionAnimData->LocomotionBS;
	CachedRunLocomotionBS = NewWeaponData->LocomotionAnimData->RunLocomotionBS;
	CachedLockOnLocomotionBS = NewWeaponData->LocomotionAnimData->LockOnLocomotionBS;
	CachedGuardLocomotionBS = NewWeaponData->LocomotionAnimData->GuardLocomotionBS;
	CachedGuardLocomotionBS_LockOn = NewWeaponData->LocomotionAnimData->GuardLocomotionBS_LockOn;
	CachedStaggerBlendSpace = NewWeaponData->HitReactionAnimData->StaggerBlendSpace;
}
