#include "Animation/RVPlayerAnimInstance.h"
#include "Component/RVComboComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Component/RVHitReactionComponent.h"
#include "Component/RVLockOnComponent.h"
#include "Component/RVSprintComponent.h"
#include "Data/RVWeaponDataAsset.h"
#include "Data/RVLocomotionAnimDataAsset.h"
#include "Data/RVHitReactionAnimDataAsset.h"
#include "KismetAnimationLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void URVPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (!IsValid(OwnerCharacter)) { return; }

    EquipmentComponent   = OwnerCharacter->FindComponentByClass<URVEquipmentComponent>();
    ComboComponent       = OwnerCharacter->FindComponentByClass<URVComboComponent>();
    CombatStateComponent = OwnerCharacter->FindComponentByClass<URVCombatStateComponent>();
    HitReactionComponent = OwnerCharacter->FindComponentByClass<URVHitReactionComponent>();
    LockOnComponent      = OwnerCharacter->FindComponentByClass<URVLockOnComponent>();
    SprintComponent      = OwnerCharacter->FindComponentByClass<URVSprintComponent>();

    ensureMsgf(IsValid(EquipmentComponent),   TEXT("[%s] EquipmentComponent missing"),   *GetNameSafe(OwnerCharacter));
    ensureMsgf(IsValid(ComboComponent),       TEXT("[%s] ComboComponent missing"),       *GetNameSafe(OwnerCharacter));
    ensureMsgf(IsValid(CombatStateComponent), TEXT("[%s] CombatStateComponent missing"), *GetNameSafe(OwnerCharacter));
    ensureMsgf(IsValid(HitReactionComponent), TEXT("[%s] HitReactionComponent missing"), *GetNameSafe(OwnerCharacter));
    ensureMsgf(IsValid(LockOnComponent),      TEXT("[%s] LockOnComponent missing"),      *GetNameSafe(OwnerCharacter));
    ensureMsgf(IsValid(SprintComponent),      TEXT("[%s] SprintComponent missing"),      *GetNameSafe(OwnerCharacter));

    if (IsValid(SprintComponent))
    {
        MaxLocomotionSpeed = SprintComponent->GetSprintSpeed();
    }

    if (IsValid(EquipmentComponent))
    {
        EquipmentComponent->OnWeaponChanged.AddDynamic(
            this, &URVPlayerAnimInstance::OnWeaponChangedHandler);

        OnWeaponChangedHandler(EquipmentComponent->GetCurrentWeaponData());
    }
}

void URVPlayerAnimInstance::NativeUninitializeAnimation()
{
    if (IsValid(EquipmentComponent))
    {
        EquipmentComponent->OnWeaponChanged.RemoveDynamic(
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

    Direction = UKismetAnimationLibrary::CalculateDirection(
        OwnerCharacter->GetVelocity(),
        OwnerCharacter->GetActorRotation());

    bIsInAir         = OwnerCharacter->GetCharacterMovement()->IsFalling();
    bIsAttacking     = ComboComponent->IsComboActive();
    bIsGuarding      = CombatStateComponent->IsInState(ERVCombatState::Guarding);
    bIsInHitReaction = CombatStateComponent->IsInState(ERVCombatState::HitReaction);
    bIsKnockedDown   = CombatStateComponent->IsInState(ERVCombatState::Knockdown);
    bIsLockedOn      = LockOnComponent->IsLockedOn();
    bIsSprinting     = SprintComponent->IsSprinting();

    StaggerDirection = HitReactionComponent->GetStaggerDirection();
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