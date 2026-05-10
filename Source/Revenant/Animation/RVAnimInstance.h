#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RVAnimInstance.generated.h"

class URVEquipmentComponent;
class URVComboComponent;
class URVCombatStateComponent;
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

    /**
     * InWorldHitDirection: world-space FROM instigator TOWARD target (normalized).
     */
    void TriggerHitReaction(const FVector& InWorldHitDirection);

protected:
    // --- Locomotion -----------------------------------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    float Speed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    float Direction;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UBlendSpace> CachedLocomotionBS;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UBlendSpace> CachedLockOnLocomotionBS;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UBlendSpace> CachedGuardLocomotionBS;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UBlendSpace> CachedGuardLocomotionBS_LockOn;

    // --- Locomotion State ----------------------------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    uint8 bIsInAir : 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    uint8 bIsAttacking : 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    uint8 bIsLockedOn : 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    uint8 bIsGuarding : 1;

    // --- Physical Reaction ---------------------------------------------------

    /**
     * Alpha for the additive hit-reaction layer in the ABP.
     * Set to 1.0 by TriggerHitReaction, then decays to 0 in NativeUpdateAnimation.
     * Wire this to the "Alpha" pin of an "Apply Additive" node in ABP.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    float HitReactionWeight;

    /**
     * Angle (CalculateDirection-style, -180 to 180) of the hit origin direction,
     * relative to this character's facing. Used by an AimOffset or blendspace
     * in the ABP to select the correct directional flinch pose.
     *
     *   0   = hit came from front
     *   180 = hit came from back
     *   90  = hit came from right side
     *  -90  = hit came from left side
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    float HitDirectionAngle;

private:
    UPROPERTY()
    TObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<URVEquipmentComponent> EquipmentComponent;

    UPROPERTY()
    TObjectPtr<URVComboComponent> ComboComponent;

    UPROPERTY()
    TObjectPtr<URVCombatStateComponent> CombatStateComponent;

    /** Rate at which HitReactionWeight decays per second. Adjust for flinch duration. */
    static constexpr float HitReactionDecayRate = 4.f; // full decay in ~0.25s

    UFUNCTION()
    void OnWeaponChangedHandler(URVWeaponDataAsset* NewWeaponData);
};