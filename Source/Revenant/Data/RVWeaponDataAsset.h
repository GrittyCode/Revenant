// Source/Revenant/Data/RVWeaponDataAsset.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "RVWeaponDataAsset.generated.h"

class URVLocomotionAnimDataAsset;
class URVPlayerCombatAnimDataAsset;
class URVHitReactionAnimDataAsset;
struct FRVWeaponStatRow;

UCLASS(BlueprintType)
class REVENANT_API URVWeaponDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
	
	// -- Stat ----------------------------------------------------------------
	
	// Points to a row in DT_WeaponStats.
	// Final hit values = WeaponStat.Base* × DT_AttackStats.Multiplier (via URVMontageStatData).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Stat")
	FDataTableRowHandle WeaponStatRowHandle;

	//--- Weapon Mesh ---------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Mesh")
	TSoftObjectPtr<UStaticMesh> WeaponMesh;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Transform")
	FTransform WeaponAttachTransform;
	
    //--- Animation -----------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<URVLocomotionAnimDataAsset> LocomotionAnimData;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<URVPlayerCombatAnimDataAsset> CombatAnimData;
	
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<URVHitReactionAnimDataAsset> HitReactionAnimData;
	
    //--- Per-Instance Montage Overrides --------------------------------------

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

    //--- Getters -------------------------------------------------------------

    const FRVWeaponStatRow* GetWeaponStatRow() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetHeavyChargeMontage() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetHeavyAttackMontage(bool bIsMax) const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetDodgeMontage() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetDodgeMontage_LockOn_F() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetDodgeMontage_LockOn_L() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetDodgeMontage_LockOn_R() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetDodgeMontage_LockOn_BL() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetDodgeMontage_LockOn_BR() const;
};