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

    OwnerCharacter->GetOnWeaponChanged().AddDynamic(
        this, &URVPlayerAnimInstance::OnWeaponChangedHandler);

    OnWeaponChangedHandler(OwnerCharacter->GetEquipmentComponent()->GetCurrentWeaponData());
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

    const UWorld* W = GetWorld();
    if (!W || !W->IsGameWorld()) { return; }

    Speed           = OwnerCharacter->GetVelocity().Size2D();
    NormalizedSpeed = FMath::Clamp(Speed / MaxLocomotionSpeed, 0.f, 1.f);
    Direction       = UKismetAnimationLibrary::CalculateDirection(
                          OwnerCharacter->GetVelocity(),
                          OwnerCharacter->GetActorRotation());

    bIsInAir         = OwnerCharacter->GetCharacterMovement()->IsFalling();
    bIsGuarding      = OwnerCharacter->IsInCombatState(ERVCombatState::Guarding);
    bIsInHitReaction = OwnerCharacter->IsInCombatState(ERVCombatState::HitReaction);
    bIsKnockedDown   = OwnerCharacter->IsInCombatState(ERVCombatState::Knockdown);
    bIsLockedOn      = OwnerCharacter->IsLockedOn();
    bIsSprinting     = OwnerCharacter->IsSprinting();
    StaggerDirection = OwnerCharacter->GetStaggerDirection();
    bIsAttacking = OwnerCharacter->IsInCombatState(
        ERVCombatState::Attacking | ERVCombatState::HeavyCharging | ERVCombatState::HeavyAttacking);
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

    if (!ensureMsgf(IsValid(NewWeaponData->LocomotionAnimData),
        TEXT("[URVPlayerAnimInstance] WeaponData '%s' has no LocomotionAnimData"),
        *GetNameSafe(NewWeaponData))) { return; }

    if (!ensureMsgf(IsValid(NewWeaponData->HitReactionAnimData),
        TEXT("[URVPlayerAnimInstance] WeaponData '%s' has no HitReactionAnimData"),
        *GetNameSafe(NewWeaponData))) { return; }

    CachedLocomotionBS             = NewWeaponData->LocomotionAnimData->LocomotionBS;
    CachedRunLocomotionBS          = NewWeaponData->LocomotionAnimData->RunLocomotionBS;
    CachedLockOnLocomotionBS       = NewWeaponData->LocomotionAnimData->LockOnLocomotionBS;
    CachedGuardLocomotionBS        = NewWeaponData->LocomotionAnimData->GuardLocomotionBS;
    CachedGuardLocomotionBS_LockOn = NewWeaponData->LocomotionAnimData->GuardLocomotionBS_LockOn;
    CachedStaggerBlendSpace        = NewWeaponData->HitReactionAnimData->StaggerBlendSpace;
}