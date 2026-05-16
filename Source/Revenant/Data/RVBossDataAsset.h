#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RVBossDataAsset.generated.h"

class URVLocomotionAnimDataAsset;
class URVCombatDataAsset;
class UAnimMontage;

// Per-phase attack montage pool. StateTree task picks randomly from Montages.
// Add as many montages as needed per phase — no fixed cap.
USTRUCT(BlueprintType)
struct FRVBossPhaseAttacks
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TArray<TObjectPtr<UAnimMontage>> Montages;
};

UCLASS()
class REVENANT_API URVBossDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    //--- Identity ------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "RV|Boss")
    FText BossName;

    //--- Locomotion ----------------------------------------------------------
    // Same class as player — URVAnimInstance reads LocomotionBS from here
    // if the boss ever uses the same ABP locomotion path.

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<URVLocomotionAnimDataAsset> LocomotionAnimData;

    //--- Phase Thresholds ----------------------------------------------------
    // HP ratio (0.0 ~ 1.0). Transition fires when HP falls below the value.

    UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Phase",
              meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Phase2Threshold = 0.65f;

    UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Phase",
              meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Phase3Threshold = 0.35f;

    //--- Phase Attack Sets ---------------------------------------------------
    // ExecutePhaseAttack() picks a random montage from the current phase pool.
    // Add or remove montages freely — no code change required.

    UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Attacks")
    FRVBossPhaseAttacks Phase1Attacks;

    UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Attacks")
    FRVBossPhaseAttacks Phase2Attacks;

    UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Attacks")
    FRVBossPhaseAttacks Phase3Attacks;

    //--- Groggy --------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Groggy")
    int32 GroggyPoiseDepletionCount = 3;

    UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Groggy")
    float GroggyDuration = 5.f;

    UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Groggy")
    TObjectPtr<UAnimMontage> GroggyStartMontage;

    UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Groggy")
    TObjectPtr<UAnimMontage> GroggyEndMontage;

    //--- Combat Data ---------------------------------------------------------
    // Attack stats (DT row) + hit reaction animations (Knockdown, GetUp, StaggerBS).

    UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Combat")
    TObjectPtr<URVCombatDataAsset> CombatData;
};