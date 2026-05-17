// Source/Revenant/Data/RVCombatAnimDataAsset.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RVPlayerCombatAnimDataAsset.generated.h"

class UAnimMontage;


UCLASS(BlueprintType)
class REVENANT_API URVPlayerCombatAnimDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    //--- Combo ---------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Combo")
    TArray<TObjectPtr<UAnimMontage>> ComboMontages;

    UAnimMontage* GetComboMontage(int32 InIndex) const;
    int32 GetMaxComboCount() const { return ComboMontages.Num(); }
    int32 FindComboMontageIndex(const UAnimMontage* InMontage) const;

    //--- Heavy Attack --------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|HeavyAttack")
    TObjectPtr<UAnimMontage> HeavyChargeMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|HeavyAttack")
    TObjectPtr<UAnimMontage> HeavyAttackMontage;

    // Played when charge completes without manual release (max charge).
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|HeavyAttack")
    TObjectPtr<UAnimMontage> MaxHeavyAttackMontage;

    //--- Dodge ---------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Dodge")
    TObjectPtr<UAnimMontage> DodgeMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Dodge|LockOn")
    TObjectPtr<UAnimMontage> LockOnDodgeMontage_F;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Dodge|LockOn")
    TObjectPtr<UAnimMontage> LockOnDodgeMontage_L;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Dodge|LockOn")
    TObjectPtr<UAnimMontage> LockOnDodgeMontage_R;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Dodge|LockOn")
    TObjectPtr<UAnimMontage> LockOnDodgeMontage_BL;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Dodge|LockOn")
    TObjectPtr<UAnimMontage> LockOnDodgeMontage_BR;

    //--- Guard ---------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Guard")
    TObjectPtr<UAnimMontage> GuardHitMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Guard")
    TObjectPtr<UAnimMontage> GuardBreakMontage;
};