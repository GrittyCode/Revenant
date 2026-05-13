// Source/Revenant/Animation/RVAnimInstance.cpp
#include "Animation/RVAnimInstance.h"
#include "Component/RVComboComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Component/RVHitReactionComponent.h"
#include "Component/RVLockOnComponent.h"
#include "Component/RVSprintComponent.h"
#include "Data/RVWeaponDataAsset.h"
#include "KismetAnimationLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void URVAnimInstance::NativeInitializeAnimation()
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
            this, &URVAnimInstance::OnWeaponChangedHandler);

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

    // --- Locomotion ----------------------------------------------------------

    Speed = OwnerCharacter->GetVelocity().Size2D();
    NormalizedSpeed = FMath::Clamp(Speed / MaxLocomotionSpeed, 0.f, 1.f);

    Direction = UKismetAnimationLibrary::CalculateDirection(
        OwnerCharacter->GetVelocity(),
        OwnerCharacter->GetActorRotation());

    // --- State ---------------------------------------------------------------

    bIsInAir         = OwnerCharacter->GetCharacterMovement()->IsFalling();
    bIsAttacking     = ComboComponent->IsComboActive();
    bIsGuarding      = CombatStateComponent->IsInState(ERVCombatState::Guarding);
    bIsInHitReaction = CombatStateComponent->IsInState(ERVCombatState::HitReaction);
    bIsKnockedDown   = CombatStateComponent->IsInState(ERVCombatState::Knockdown);
    bIsLockedOn      = LockOnComponent->IsLockedOn();
    bIsSprinting     = SprintComponent->IsSprinting();

    StaggerDirection = HitReactionComponent->GetStaggerDirection();
}

void URVAnimInstance::OnWeaponChangedHandler(URVWeaponDataAsset* NewWeaponData)
{
    CachedLocomotionBS             = IsValid(NewWeaponData) ? NewWeaponData->GetLocomotionBS()             : nullptr;
    CachedRunLocomotionBS          = IsValid(NewWeaponData) ? NewWeaponData->GetRunLocomotionBS()          : nullptr;
    CachedLockOnLocomotionBS       = IsValid(NewWeaponData) ? NewWeaponData->GetLockOnLocomotionBS()       : nullptr;
    CachedGuardLocomotionBS        = IsValid(NewWeaponData) ? NewWeaponData->GetGuardLocomotionBS()        : nullptr;
    CachedGuardLocomotionBS_LockOn = IsValid(NewWeaponData) ? NewWeaponData->GetGuardLocomotionBS_LockOn() : nullptr;
    CachedStaggerBlendSpace        = IsValid(NewWeaponData) ? NewWeaponData->GetStaggerBlendSpace()        : nullptr;
}