#include "Animation/RVPlayerAnimInstance.h"
#include "Data/RVWeaponDataAsset.h"
#include "Data/RVLocomotionAnimDataAsset.h"
#include "Data/RVHitReactionAnimDataAsset.h"
#include "Component/RVCombatStateComponent.h"
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

    OwnerCharacter->GetOnWeaponChanged().AddDynamic(
        this, &URVPlayerAnimInstance::OnWeaponChangedHandler);

    OnWeaponChangedHandler(OwnerCharacter->GetCurrentWeaponData());
}

void URVPlayerAnimInstance::NativeUninitializeAnimation()
{
    if (IsValid(OwnerCharacter))
    {
        OwnerCharacter->GetOnWeaponChanged().RemoveDynamic(
            this, &URVPlayerAnimInstance::OnWeaponChangedHandler);
    }

    Super::NativeUninitializeAnimation();
}

void URVPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!IsValid(OwnerCharacter)) { return; }

    Speed           = OwnerCharacter->GetVelocity().Size2D();
    NormalizedSpeed = FMath::Clamp(Speed / MaxLocomotionSpeed, 0.f, 1.f);
    Direction       = UKismetAnimationLibrary::CalculateDirection(
                          OwnerCharacter->GetVelocity(),
                          OwnerCharacter->GetActorRotation());

    bIsInAir         = OwnerCharacter->GetCharacterMovement()->IsFalling();
    bIsAttacking     = OwnerCharacter->IsComboActive();
    bIsGuarding      = OwnerCharacter->IsInCombatState(ERVCombatState::Guarding);
    bIsInHitReaction = OwnerCharacter->IsInCombatState(ERVCombatState::HitReaction);
    bIsKnockedDown   = OwnerCharacter->IsInCombatState(ERVCombatState::Knockdown);
    bIsLockedOn      = OwnerCharacter->IsLockedOn();
    bIsSprinting     = OwnerCharacter->IsSprinting();
    StaggerDirection = OwnerCharacter->GetStaggerDirection();
}

void URVPlayerAnimInstance::OnWeaponChangedHandler(URVWeaponDataAsset* NewWeaponData)
{
    if (!IsValid(NewWeaponData))
    {
        CachedLocomotionBS             = nullptr;
        CachedRunLocomotionBS          = nullptr;
        CachedLockOnLocomotionBS       = nullptr;
        CachedGuardLocomotionBS        = nullptr;
        CachedGuardLocomotionBS_LockOn = nullptr;
        CachedStaggerBlendSpace        = nullptr;
        return;
    }

    CachedLocomotionBS             = NewWeaponData->LocomotionAnimData->LocomotionBS;
    CachedRunLocomotionBS          = NewWeaponData->LocomotionAnimData->RunLocomotionBS;
    CachedLockOnLocomotionBS       = NewWeaponData->LocomotionAnimData->LockOnLocomotionBS;
    CachedGuardLocomotionBS        = NewWeaponData->LocomotionAnimData->GuardLocomotionBS;
    CachedGuardLocomotionBS_LockOn = NewWeaponData->LocomotionAnimData->GuardLocomotionBS_LockOn;
    CachedStaggerBlendSpace        = NewWeaponData->HitReactionAnimData->StaggerBlendSpace;
}