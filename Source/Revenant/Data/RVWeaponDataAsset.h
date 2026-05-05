#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RVWeaponDataAsset.generated.h"

class UAnimMontage;
class UBlendSpace;
class URVWeaponStyleDataAsset;

/**
 * Per-instance weapon DataAsset.
 * Owns combat values directly.
 * All animation and locomotion assets are resolved from WeaponStyle.
 */
UCLASS(BlueprintType)
class REVENANT_API URVWeaponDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    /** Shared animation and locomotion style for this weapon. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|WeaponStyle")
    TObjectPtr<URVWeaponStyleDataAsset> WeaponStyle;

    // --- Combat Values (owned per instance) ----------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Attack")
    float AttackDamage = 30.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Attack")
    float AttackRadius = 40.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Attack")
    float AttackStaminaCost = 20.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Dodge")
    float DodgeStaminaCost = 30.f;

    // --- Combo Override Group ------------------------------------------------
    // All three fields must be set together.
    // Enable bOverrideCombo to activate this group.

    /** Enable to override all combo fields for this instance. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|Combo",
              meta = (InlineEditConditionToggle))
    uint8 bOverrideCombo : 1;

    /** Replaces WeaponStyle->AttackMontage. Set with MaxComboCount and SectionNames. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|Combo",
              meta = (EditCondition = "bOverrideCombo"))
    TObjectPtr<UAnimMontage> OverrideAttackMontage;

    /** Replaces WeaponStyle->MaxComboCount. Must match OverrideComboSectionNames.Num(). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|Combo",
              meta = (EditCondition = "bOverrideCombo", ClampMin = "1"))
    int32 OverrideMaxComboCount = 1;

    /** Replaces WeaponStyle->ComboSectionNames. Num() must match OverrideMaxComboCount. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|Combo",
              meta = (EditCondition = "bOverrideCombo"))
    TArray<FName> OverrideComboSectionNames;

    // --- Dodge Override ------------------------------------------------------

    /** Enable to override dodge montage for this instance. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|Dodge",
              meta = (InlineEditConditionToggle))
    uint8 bOverrideDodgeMontage : 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|Dodge",
              meta = (EditCondition = "bOverrideDodgeMontage"))
    TObjectPtr<UAnimMontage> OverrideDodgeMontage;

    // --- Getters (fallback logic — always use these for asset fields) ---------

    /** Returns override attack montage if bOverrideCombo, else WeaponStyle->AttackMontage. */
    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetAttackMontage() const;

    /** Returns override dodge montage if bOverrideDodgeMontage, else WeaponStyle->DodgeMontage. */
    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetDodgeMontage() const;

    /** Returns WeaponStyle->GuardBreakMontage. */
    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetGuardBreakMontage() const;

    /** Returns WeaponStyle->GuardHitMontage. */
    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetGuardHitMontage() const;

    /** Returns WeaponStyle->LocomotionBS. */
    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UBlendSpace* GetLocomotionBS() const;

    /** Returns WeaponStyle->LockOnLocomotionBS. */
    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UBlendSpace* GetLockOnLocomotionBS() const;

    /** Returns WeaponStyle->GuardLocomotionBS. */
    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UBlendSpace* GetGuardLocomotionBS() const;

    /** Returns WeaponStyle->GuardLocomotionBS_LockOn. */
    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UBlendSpace* GetGuardLocomotionBS_LockOn() const;

    /** Returns OverrideMaxComboCount if bOverrideCombo, else WeaponStyle->MaxComboCount. */
    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    int32 GetMaxComboCount() const;

    /** Returns OverrideComboSectionNames if bOverrideCombo, else WeaponStyle->ComboSectionNames. */
    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    const TArray<FName>& GetComboSectionNames() const;
};