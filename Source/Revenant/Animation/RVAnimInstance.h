#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RVAnimInstance.generated.h"

class URVEquipmentComponent;
class URVComboComponent;
class URVCombatComponent;
class URVWeaponDataAsset;
class UBlendSpace;

UCLASS()
class REVENANT_API URVAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUninitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
    // --- Locomotion -----------------------------------------------------------

    // XY plane speed only — vertical velocity excluded to avoid blending artifacts during jumps.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    float Speed;

    // -180 ~ 180 degrees, 0 = forward, 90 = right, -90 = left, -180 = back
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    float Direction;

    // Updated via OnWeaponChanged — drives ABP default locomotion Blendspace Player.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UBlendSpace> CachedLocomotionBS;

    // Updated via OnWeaponChanged — drives ABP lock-on locomotion Blendspace Player.
    // Direction + Speed; strafe mode. bOrientRotationToMovement off while locked on.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UBlendSpace> CachedLockOnLocomotionBS;

    // Updated via OnWeaponChanged — drives ABP guard locomotion (no lock-on).
    // Speed-only; character rotates via bOrientRotationToMovement.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UBlendSpace> CachedGuardLocomotionBS;

    // Updated via OnWeaponChanged — drives ABP guard locomotion (lock-on active).
    // Direction + Speed; full 6-direction strafe. nullptr until Phase 4 asset is assigned.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UBlendSpace> CachedGuardLocomotionBS_LockOn;

    // --- State ----------------------------------------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    uint8 bIsInAir : 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    uint8 bIsAttacking : 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    uint8 bIsLockedOn : 1;

    // Set by URVCombatComponent::StartGuard / EndGuard — drives ABP guard branch.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    uint8 bIsGuarding : 1;

private:
    UPROPERTY()
    TObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<URVEquipmentComponent> EquipmentComponent;

    UPROPERTY()
    TObjectPtr<URVComboComponent> ComboComponent;

    UPROPERTY()
    TObjectPtr<URVCombatComponent> CombatComponent;

    // Bound to URVEquipmentComponent::OnWeaponChanged
    UFUNCTION()
    void OnWeaponChangedHandler(URVWeaponDataAsset* NewWeaponData);
};