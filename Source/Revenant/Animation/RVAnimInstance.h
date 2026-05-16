#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RVAnimInstance.generated.h"

class URVEquipmentComponent;
class URVComboComponent;
class URVCombatStateComponent;
class URVHitReactionComponent;
class URVLockOnComponent;
class URVSprintComponent;
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
    // --- Locomotion ----------------------------------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    float Speed;

    // Speed / SprintSpeed (0~1). BS input — stays accurate regardless of sprint state.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    float NormalizedSpeed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    float Direction;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UBlendSpace> CachedLocomotionBS;

    // Sprint state only — Default mode. Switched via bIsSprinting in ABP.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UBlendSpace> CachedRunLocomotionBS;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UBlendSpace> CachedLockOnLocomotionBS;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UBlendSpace> CachedGuardLocomotionBS;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UBlendSpace> CachedGuardLocomotionBS_LockOn;

    // --- State ---------------------------------------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    uint8 bIsInAir : 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    uint8 bIsAttacking : 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    uint8 bIsLockedOn : 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    uint8 bIsSprinting : 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    uint8 bIsGuarding : 1;

    // True during stagger (timer-based). Drives Grounded → HitReaction transition.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    uint8 bIsInHitReaction : 1;

    // True during knockdown + get-up montage. Drives HitReaction → Knockdown transition.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    uint8 bIsKnockedDown : 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UBlendSpace> CachedStaggerBlendSpace;

    // Polled from URVHitReactionComponent each frame.
    // Character-local hit angle (-180~180) — Stagger BS Y-axis input.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    float StaggerDirection;

private:
    UPROPERTY()
    TObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<URVEquipmentComponent> EquipmentComponent;

    UPROPERTY()
    TObjectPtr<URVComboComponent> ComboComponent;

    UPROPERTY()
    TObjectPtr<URVCombatStateComponent> CombatStateComponent;

    UPROPERTY()
    TObjectPtr<URVHitReactionComponent> HitReactionComponent;

    UPROPERTY()
    TObjectPtr<URVLockOnComponent> LockOnComponent;

    UPROPERTY()
    TObjectPtr<URVSprintComponent> SprintComponent;

    // Cached at init from SprintComponent — fixed denominator for NormalizedSpeed.
    float MaxLocomotionSpeed = 1.f;

    UFUNCTION()
    void OnWeaponChangedHandler(URVWeaponDataAsset* NewWeaponData);
};