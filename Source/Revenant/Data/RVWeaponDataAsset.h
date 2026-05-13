#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "RVWeaponDataAsset.generated.h"

class UAnimMontage;
class UBlendSpace;
class USkeletalMesh;
class URVWeaponAnimationDataAsset;
struct FRVWeaponStatRow;

/**
 * Layer 2 — Weapon Instance.
 * Represents one specific weapon. References an AnimationDataAsset for all
 * shared moveset data, owns its own stat row and mesh.
 */
UCLASS(BlueprintType)
class REVENANT_API URVWeaponDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    //--- Animation Set Reference ---------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<URVWeaponAnimationDataAsset> AnimationDataAsset;

    //--- Base Stats ----------------------------------------------------------
    // Points to a row in DT_WeaponStats.
    // Final hit values = WeaponStat.Base* x AttackStatRow.Multiplier.

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Combat")
    FDataTableRowHandle WeaponStatRowHandle;

    const FRVWeaponStatRow* GetWeaponStatRow() const;

    //--- Weapon Mesh ---------------------------------------------------------
    // Loaded and attached by URVEquipmentComponent in Phase 4.
    // Soft reference — asset path stored, loaded on demand.
    // WeaponRoot / WeaponTip sockets on this mesh define attack capsule dimensions.

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Mesh")
    TSoftObjectPtr<USkeletalMesh> WeaponMesh;

    //--- Per-Instance Montage Overrides --------------------------------------
    // Use only when this specific weapon needs a different montage from its animation set.

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|HeavyAttack",
              meta = (InlineEditConditionToggle))
    uint8 bOverrideHeavyChargeMontage : 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|HeavyAttack",
              meta = (EditCondition = "bOverrideHeavyChargeMontage"))
    TObjectPtr<UAnimMontage> OverrideHeavyChargeMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|HeavyAttack",
              meta = (InlineEditConditionToggle))
    uint8 bOverrideHeavyAttackMontage : 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|HeavyAttack",
              meta = (EditCondition = "bOverrideHeavyAttackMontage"))
    TObjectPtr<UAnimMontage> OverrideHeavyAttackMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|HeavyAttack",
              meta = (EditCondition = "bOverrideHeavyAttackMontage"))
    TObjectPtr<UAnimMontage> OverrideMaxHeavyAttackMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|Dodge",
              meta = (InlineEditConditionToggle))
    uint8 bOverrideDodgeMontage : 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|Dodge",
              meta = (EditCondition = "bOverrideDodgeMontage"))
    TObjectPtr<UAnimMontage> OverrideDodgeMontage;

    //--- Getters (all route through AnimationDataAsset) ----------------------

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetComboMontage(int32 InIndex) const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    int32 GetMaxComboCount() const;

    int32 FindComboMontageIndex(const UAnimMontage* InMontage) const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetHeavyChargeMontage() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetHeavyAttackMontage(bool bIsMax) const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetDodgeMontage() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetGuardBreakMontage() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetGuardHitMontage() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetGroggyMontage() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetKnockdownMontage() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetGetUpMontage() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UBlendSpace* GetStaggerBlendSpace() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UBlendSpace* GetLocomotionBS() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UBlendSpace* GetRunLocomotionBS() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UBlendSpace* GetLockOnLocomotionBS() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UBlendSpace* GetGuardLocomotionBS() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UBlendSpace* GetGuardLocomotionBS_LockOn() const;
};