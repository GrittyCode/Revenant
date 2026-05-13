#include "Animation/RVAnimInstance.h"
#include "Component/RVComboComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Component/RVHitReactionComponent.h"
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

    Direction = UKismetAnimationLibrary::CalculateDirection(
        OwnerCharacter->GetVelocity(),
        OwnerCharacter->GetActorRotation());

    // --- State ---------------------------------------------------------------

    bIsInAir         = OwnerCharacter->GetCharacterMovement()->IsFalling();
    bIsAttacking     = IsValid(ComboComponent)       ? ComboComponent->IsComboActive()                               : false;
    bIsGuarding      = IsValid(CombatStateComponent) ? CombatStateComponent->IsInState(ERVCombatState::Guarding)     : false;
    bIsInHitReaction = IsValid(CombatStateComponent) ? CombatStateComponent->IsInState(ERVCombatState::HitReaction)  : false;
    bIsKnockedDown   = IsValid(CombatStateComponent) ? CombatStateComponent->IsInState(ERVCombatState::Knockdown)    : false;
    bIsLockedOn      = false;

    StaggerDirection = IsValid(HitReactionComponent) ? HitReactionComponent->GetStaggerDirection() : 0.f;
}

void URVAnimInstance::OnWeaponChangedHandler(URVWeaponDataAsset* NewWeaponData)
{
    CachedLocomotionBS             = IsValid(NewWeaponData) ? NewWeaponData->GetLocomotionBS()             : nullptr;
    CachedLockOnLocomotionBS       = IsValid(NewWeaponData) ? NewWeaponData->GetLockOnLocomotionBS()       : nullptr;
    CachedGuardLocomotionBS        = IsValid(NewWeaponData) ? NewWeaponData->GetGuardLocomotionBS()        : nullptr;
    CachedGuardLocomotionBS_LockOn = IsValid(NewWeaponData) ? NewWeaponData->GetGuardLocomotionBS_LockOn() : nullptr;
    CachedStaggerBlendSpace        = IsValid(NewWeaponData) ? NewWeaponData->GetStaggerBlendSpace()        : nullptr;
}