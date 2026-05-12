// Source/Revenant/Data/RVWeaponAnimationDataAsset.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RVWeaponAnimationDataAsset.generated.h"

class UAnimMontage;
class UBlendSpace;


/**
 * Layer 1 — Weapon Animation Set.
 * Defines a moveset: all montages, blendspaces, and the weapon category this style belongs to.
 * Multiple weapon instances (URVWeaponDataAsset) can reference the same animation set
 */
UCLASS(BlueprintType)
class REVENANT_API URVWeaponAnimationDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    //--- Combo Montages ------------------------------------------------------
    // Each montage carries its own stat multiplier row via URVMontageStatData (AssetUserData).

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Combo")
    TArray<TObjectPtr<UAnimMontage>> ComboMontages;

    UAnimMontage* GetComboMontage(int32 InIndex) const;
    int32 GetMaxComboCount() const { return ComboMontages.Num(); }

    // Returns the index of InMontage in ComboMontages, or INDEX_NONE if not found.
    int32 FindComboMontageIndex(const UAnimMontage* InMontage) const;

    //--- Attack Montages -----------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UAnimMontage> HeavyChargeMontage;

    // Played on manual release.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UAnimMontage> HeavyAttackMontage;

    // Played on auto-release (max charge).
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UAnimMontage> MaxHeavyAttackMontage;

    // Single forward roll — character is pre-rotated to dodge direction before play.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UAnimMontage> DodgeMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UAnimMontage> GuardBreakMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UAnimMontage> GuardHitMontage;

    //--- Hit Reaction --------------------------------------------------------

    // Direction axis: -180 to 180. ABP samples at HitDirectionAngle during HitReaction state.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|HitReaction")
    TObjectPtr<UBlendSpace> StaggerBlendSpace;

    // Must contain "Loop" section (looping) and "End" section (plays once).
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|HitReaction")
    TObjectPtr<UAnimMontage> GroggyMontage;

    // Transitions to GetUpMontage on blend-out.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|HitReaction")
    TObjectPtr<UAnimMontage> KnockdownMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|HitReaction")
    TObjectPtr<UAnimMontage> GetUpMontage;

    //--- Locomotion ----------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Locomotion")
    TObjectPtr<UBlendSpace> LocomotionBS;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Locomotion")
    TObjectPtr<UBlendSpace> LockOnLocomotionBS;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Locomotion")
    TObjectPtr<UBlendSpace> GuardLocomotionBS;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Locomotion")
    TObjectPtr<UBlendSpace> GuardLocomotionBS_LockOn;
};