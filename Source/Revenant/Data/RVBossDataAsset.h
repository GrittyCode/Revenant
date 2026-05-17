// Source/Revenant/Data/RVBossDataAsset.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "RVBossDataAsset.generated.h"

class URVLocomotionAnimDataAsset;
class URVHitReactionAnimDataAsset;
class UAnimMontage;
struct FRVEnemyStatRow;

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

    //--- Enemy Stats ---------------------------------------------------------
    // All numeric tuning lives in DT_EnemyStats.
    // Designers edit the CSV — no DataAsset changes needed for balance work.

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Boss|Stats")
    FDataTableRowHandle EnemyStatRowHandle;

    const FRVEnemyStatRow* GetEnemyStatRow() const;

    //--- Locomotion ----------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<URVLocomotionAnimDataAsset> LocomotionAnimData;

    //--- Phase Thresholds ----------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Phase",
              meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Phase2Threshold = 0.65f;

    UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Phase",
              meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Phase3Threshold = 0.35f;

    //--- Phase Attack Sets ---------------------------------------------------

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
	
    //--- Combat Data ---------------------------------------------------------
    // Hit reaction animations (StaggerBS, KnockdownMontage, GetUpMontage).

    UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Hit")
    TObjectPtr<URVHitReactionAnimDataAsset> HitReactionAnimData;
};