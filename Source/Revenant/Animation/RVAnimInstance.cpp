// Source/Revenant/Animation/RVAnimInstance.cpp
#include "Animation/RVAnimInstance.h"
#include "Component/RVComboComponent.h"
#include "Component/RVCombatStateComponent.h"
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

    EquipmentComponent   = OwnerCharacter->FindComponentByClass<URVEquipmentComponent>();
    ComboComponent       = OwnerCharacter->FindComponentByClass<URVComboComponent>();
    CombatStateComponent = OwnerCharacter->FindComponentByClass<URVCombatStateComponent>();

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

    // --- Locomotion -----------------------------------------------------------

    Speed = OwnerCharacter->GetVelocity().Size2D();

    Direction = UKismetAnimationLibrary::CalculateDirection(
        OwnerCharacter->GetVelocity(),
        OwnerCharacter->GetActorRotation());

    // --- State ----------------------------------------------------------------

    bIsInAir     = OwnerCharacter->GetCharacterMovement()->IsFalling();
    bIsAttacking = IsValid(ComboComponent) ? ComboComponent->IsComboActive() : false;
    bIsGuarding  = IsValid(CombatStateComponent) ? CombatStateComponent->IsInState(ERVCombatState::Guarding) : false;
    bIsLockedOn  = false;

    // --- Physical Reaction Decay ---------------------------------------------
    //
    // HitReactionWeight is set to 1.0 by TriggerHitReaction and decays linearly
    // here each frame. The ABP uses this as the alpha for the additive flinch layer.
    // HitReactionDecayRate controls how quickly the flinch blends out (default: ~0.25s).

    if (HitReactionWeight > 0.f)
    {
        HitReactionWeight = FMath::Max(0.f, HitReactionWeight - DeltaSeconds * HitReactionDecayRate);
    }
}

// --- Physical Reaction -------------------------------------------------------

void URVAnimInstance::TriggerHitReaction(const FVector& InWorldHitDirection)
{
    if (!IsValid(OwnerCharacter)) { return; }

    // Reset weight to full — ABP additive layer immediately snaps to flinch pose
    // and decays smoothly in subsequent NativeUpdateAnimation calls.
    HitReactionWeight = 1.0f;

    // Compute the angle of the hit ORIGIN (where the hit came from) relative to
    // this character's facing. The ABP AimOffset uses this to blend the correct
    // directional flinch pose (front / back / left / right).
    //
    // InWorldHitDirection is from instigator TOWARD target, so negate it to get
    // the direction pointing from target BACK to instigator (the origin of the hit).
    const FVector HitOriginDir = -InWorldHitDirection;
    HitDirectionAngle = UKismetAnimationLibrary::CalculateDirection(
        HitOriginDir,
        OwnerCharacter->GetActorRotation()
    );
}

void URVAnimInstance::OnWeaponChangedHandler(URVWeaponDataAsset* NewWeaponData)
{
    CachedLocomotionBS             = IsValid(NewWeaponData) ? NewWeaponData->GetLocomotionBS()             : nullptr;
    CachedLockOnLocomotionBS       = IsValid(NewWeaponData) ? NewWeaponData->GetLockOnLocomotionBS()       : nullptr;
    CachedGuardLocomotionBS        = IsValid(NewWeaponData) ? NewWeaponData->GetGuardLocomotionBS()        : nullptr;
    CachedGuardLocomotionBS_LockOn = IsValid(NewWeaponData) ? NewWeaponData->GetGuardLocomotionBS_LockOn() : nullptr;
}