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

    // --- Combat Values ---------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Attack")
    float AttackDamage = 30.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Attack")
    float AttackRadius = 40.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Attack")
    float AttackStaminaCost = 20.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Dodge")
    float DodgeStaminaCost = 30.f;

    // --- Poise Damage ---------------------------------------------------------

    /**
     * Poise damage dealt per hit with this weapon.
     * Accumulated in URVAttributeComponent. When CurrentPoise reaches 0,
     * URVHitReactionComponent triggers Stagger / Groggy / Knockdown.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Poise")
    float PoiseDamage = 30.f;

    /**
     * If true, a max-charge heavy attack (AutoRelease tier) forces Knockdown
     * on the target regardless of their remaining poise.
     * Set for greatswords and other high-impact weapons.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Poise")
    uint8 bHeavyAttackForceKnockdown : 1;

    // --- Heavy Attack Values ---------------------------------------------------

    /** Damage applied on manual release (player released before MaxChargeTime). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|HeavyAttack")
    float HeavyAttackDamage = 60.f;

    /** Damage applied on auto-release (held to MaxChargeTime — maximum charge). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|HeavyAttack")
    float HeavyAttackDamage_Max = 110.f;

    /** Stamina consumed when heavy attack starts charging. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|HeavyAttack")
    float HeavyAttackStaminaCost = 40.f;

    // --- Combo Override Group --------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|Combo",
              meta = (InlineEditConditionToggle))
    uint8 bOverrideCombo : 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|Combo",
              meta = (EditCondition = "bOverrideCombo"))
    TObjectPtr<UAnimMontage> OverrideAttackMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|Combo",
              meta = (EditCondition = "bOverrideCombo", ClampMin = "1"))
    int32 OverrideMaxComboCount = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|Combo",
              meta = (EditCondition = "bOverrideCombo"))
    TArray<FName> OverrideComboSectionNames;

    // --- Heavy Charge Override -------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|HeavyAttack",
              meta = (InlineEditConditionToggle))
    uint8 bOverrideHeavyChargeMontage : 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|HeavyAttack",
              meta = (EditCondition = "bOverrideHeavyChargeMontage"))
    TObjectPtr<UAnimMontage> OverrideHeavyChargeMontage;

    // --- Heavy Release Override ------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|HeavyAttack",
              meta = (EditCondition = "bOverrideHeavyAttackMontage",
                      InlineEditConditionToggle))
    uint8 bOverrideHeavyAttackMontage : 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|HeavyAttack",
              meta = (EditCondition = "bOverrideHeavyAttackMontage"))
    TObjectPtr<UAnimMontage> OverrideHeavyAttackMontage;

    // --- Dodge Override -------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|Dodge",
              meta = (InlineEditConditionToggle))
    uint8 bOverrideDodgeMontage : 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Override|Dodge",
              meta = (EditCondition = "bOverrideDodgeMontage"))
    TObjectPtr<UAnimMontage> OverrideDodgeMontage;

    // --- Getters (fallback logic — always use these for asset fields) ----------

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetAttackMontage() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetHeavyChargeMontage() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetHeavyAttackMontage() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetDodgeMontage() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetGuardBreakMontage() const;

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    UAnimMontage* GetGuardHitMontage() const;

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

    UFUNCTION(BlueprintCallable, Category = "RV|WeaponData")
    const TArray<FName>& GetComboSectionNames() const;
};