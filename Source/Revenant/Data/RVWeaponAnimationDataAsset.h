#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RVWeaponAnimationDataAsset.generated.h"

class UAnimMontage;
class UBlendSpace;

/**
 * Layer 1 — Weapon Animation Set.
 * Holds moveset animations and locomotion blendspaces shared across weapon instances of the same style.
 * Hit reaction animations (Stagger, Knockdown, GetUp) live in URVCombatDataAsset.
 */
UCLASS(BlueprintType)
class REVENANT_API URVWeaponAnimationDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    //--- Combo ---------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Combo")
    TArray<TObjectPtr<UAnimMontage>> ComboMontages;

    UAnimMontage* GetComboMontage(int32 InIndex) const;
    int32 GetMaxComboCount() const { return ComboMontages.Num(); }
    int32 FindComboMontageIndex(const UAnimMontage* InMontage) const;

    //--- Attack Montages -----------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UAnimMontage> HeavyChargeMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UAnimMontage> HeavyAttackMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UAnimMontage> MaxHeavyAttackMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UAnimMontage> DodgeMontage;

    //--- Lock-on Directional Dodge -------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation|LockOnDodge")
    TObjectPtr<UAnimMontage> LockOnDodgeMontage_F;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation|LockOnDodge")
    TObjectPtr<UAnimMontage> LockOnDodgeMontage_L;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation|LockOnDodge")
    TObjectPtr<UAnimMontage> LockOnDodgeMontage_R;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation|LockOnDodge")
    TObjectPtr<UAnimMontage> LockOnDodgeMontage_BL;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation|LockOnDodge")
    TObjectPtr<UAnimMontage> LockOnDodgeMontage_BR;

    //--- Guard ---------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UAnimMontage> GuardBreakMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UAnimMontage> GuardHitMontage;

    //--- Groggy (player reaction to boss groggy phase) -----------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UAnimMontage> GroggyMontage;

    //--- Locomotion ----------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Locomotion")
    TObjectPtr<UBlendSpace> LocomotionBS;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Locomotion")
    TObjectPtr<UBlendSpace> RunLocomotionBS;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Locomotion")
    TObjectPtr<UBlendSpace> LockOnLocomotionBS;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Locomotion")
    TObjectPtr<UBlendSpace> GuardLocomotionBS;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Locomotion")
    TObjectPtr<UBlendSpace> GuardLocomotionBS_LockOn;
};