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
 * Animation and locomotion assets fall back to WeaponStyle defaults via getter functions.
 * Combo assets can be overridden as a group using bOverrideCombo.
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

    // --- Montage Override Group ----------------------------------------------

    /** Enable to override dodge montage for this instance. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|Animation",
              meta = (InlineEditConditionToggle))
    uint8 bOverrideDodgeMontage : 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|Animation",
              meta = (EditCondition = "bOverrideDodgeMontage"))
    TObjectPtr<UAnimMontage> OverrideDodgeMontage;

    /** Enable to override guard break montage for this instance. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|Animation",
              meta = (InlineEditConditionToggle))
    uint8 bOverrideGuardBreakMontage : 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|Animation",
              meta = (EditCondition = "bOverrideGuardBreakMontage"))
    TObjectPtr<UAnimMontage> OverrideGuardBreakMontage;

    /** Enable to override guard hit montage for this instance. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|Animation",
              meta = (InlineEditConditionToggle))
    uint8 bOverrideGuardHitMontage : 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|Animation",
              meta = (EditCondition = "bOverrideGuardHitMontage"))
    TObjectPtr<UAnimMontage> OverrideGuardHitMontage;

    // --- Locomotion Override Group -------------------------------------------

    /** Enable to override locomotion blendspace for this instance. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|Locomotion",
              meta = (InlineEditConditionToggle))
    uint8 bOverrideLocomotionBS : 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|Locomotion",
              meta = (EditCondition = "bOverrideLocomotionBS"))
    TObjectPtr<UBlendSpace> OverrideLocomotionBS;

    /** Enable to override guard locomotion blendspace for this instance. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|Locomotion",
              meta = (InlineEditConditionToggle))
    uint8 bOverrideGuardLocomotionBS : 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|Locomotion",
              meta = (EditCondition = "bOverrideGuardLocomotionBS"))
    TObjectPtr<UBlendSpace> OverrideGuardLocomotionBS;

    // --- Getters (fallback logic — always use these for asset fields) ---------

    /** Returns override attack montage if bOverrideCombo, else WeaponStyle->AttackMontage. */
    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetAttackMontage() const;

    /** Returns override dodge montage if bOverrideDodgeMontage, else WeaponStyle->DodgeMontage. */
    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetDodgeMontage() const;

    /** Returns override guard break montage if bOverrideGuardBreakMontage, else WeaponStyle default. */
    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetGuardBreakMontage() const;

    /** Returns override guard hit montage if bOverrideGuardHitMontage, else WeaponStyle default. */
    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetGuardHitMontage() const;

    /** Returns override locomotion BS if bOverrideLocomotionBS, else WeaponStyle->LocomotionBS. */
    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UBlendSpace* GetLocomotionBS() const;

    /** Returns override guard locomotion BS if bOverrideGuardLocomotionBS, else WeaponStyle default. */
    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UBlendSpace* GetGuardLocomotionBS() const;

    /** Returns OverrideMaxComboCount if bOverrideCombo, else WeaponStyle->MaxComboCount. */
    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    int32 GetMaxComboCount() const;

    /** Returns OverrideComboSectionNames if bOverrideCombo, else WeaponStyle->ComboSectionNames. */
    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    const TArray<FName>& GetComboSectionNames() const;
};