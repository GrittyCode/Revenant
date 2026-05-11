// Source/Revenant/Data/RVWeaponDataAsset.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "RVWeaponDataAsset.generated.h"

class UAnimMontage;
class UBlendSpace;
class URVWeaponAnimationDataAsset;
struct FRVWeaponStatRow;

UCLASS(BlueprintType)
class REVENANT_API URVWeaponDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<URVWeaponAnimationDataAsset> AnimationDataAsset;

    //--- Weapon Base Stats ---------------------------------------------------
    // Points to a row in DT_WeaponStats.
    // Final hit values = WeaponStat.Base* × AttackStatRow.Multiplier.

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Combat")
    FDataTableRowHandle WeaponStatRowHandle;

    const FRVWeaponStatRow* GetWeaponStatRow() const;

    //--- Per-Instance Values -------------------------------------------------

    // Capsule width for attack sweep. Height is derived from WeaponRoot→WeaponTip distance.
    // Kept in editor — physical property of the weapon mesh, not derivable from CSV.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Combat")
    float AttackRadius = 40.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Combat")
    float DodgeStaminaCost = 30.f;

    //--- Heavy Attack Montage Override ---------------------------------------

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

    //--- Dodge Montage Override ----------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|Dodge",
              meta = (InlineEditConditionToggle))
    uint8 bOverrideDodgeMontage : 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|Dodge",
              meta = (EditCondition = "bOverrideDodgeMontage"))
    TObjectPtr<UAnimMontage> OverrideDodgeMontage;

    //--- Montage Getters -----------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetComboMontage(int32 InIndex) const;

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

    //--- Locomotion Getters --------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UBlendSpace* GetLocomotionBS() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UBlendSpace* GetLockOnLocomotionBS() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UBlendSpace* GetGuardLocomotionBS() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UBlendSpace* GetGuardLocomotionBS_LockOn() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    int32 GetMaxComboCount() const;
};