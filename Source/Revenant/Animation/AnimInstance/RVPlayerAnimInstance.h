#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RVPlayerAnimInstance.generated.h"

class URVWeaponDataAsset;
class UBlendSpace;
class ARVCharacterPlayer;

UCLASS()
class REVENANT_API URVPlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUninitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
    //--- Locomotion ----------------------------------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    float Speed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    float NormalizedSpeed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    float Direction;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UBlendSpace> CachedLocomotionBS;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UBlendSpace> CachedRunLocomotionBS;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UBlendSpace> CachedLockOnLocomotionBS;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UBlendSpace> CachedGuardLocomotionBS;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UBlendSpace> CachedGuardLocomotionBS_LockOn;

    //--- State ---------------------------------------------------------------

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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    uint8 bIsInHitReaction : 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    uint8 bIsKnockedDown : 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UBlendSpace> CachedStaggerBlendSpace;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    float StaggerDirection;

private:
    // All state queries go through ARVCharacterPlayer facade — no cached component pointers.
    UPROPERTY()
    TObjectPtr<ARVCharacterPlayer> OwnerCharacter;

    float MaxLocomotionSpeed = 1.f;

    UFUNCTION()
    void OnWeaponChangedHandler(URVWeaponDataAsset* NewWeaponData);
};