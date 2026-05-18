#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "RVBossDataAsset.generated.h"

class URVLocomotionAnimDataAsset;
class URVHitReactionAnimDataAsset;
class UAnimMontage;
struct FRVEnemyStatRow;

// One attack pattern = an ordered sequence of montages played as a combo chain.
// Each montage carries its own URVMontageStatData — per-hit damage is independent.
USTRUCT(BlueprintType)
struct FRVBossAttackPattern
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TObjectPtr<UAnimMontage>> ComboMontages;
};

// One phase's full repertoire — a random pattern is selected each attack cycle.
USTRUCT(BlueprintType)
struct FRVBossPhaseAttacks
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FRVBossAttackPattern> Patterns;
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
	
	//--- Phase Thresholds ----------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Phase",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Phase2Threshold = 0.70f;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Phase",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Phase3Threshold = 0.40f;

	//--- Phase Attack Sets ---------------------------------------------------
	// Each phase holds N patterns. One pattern is selected at random per attack cycle.
	// Within a pattern, ComboMontages are played in order (chain).

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
	float GroggyDuration = 4.f;
	
	//--- Animation Assets ----------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|AnimationAsset")
	TObjectPtr<URVLocomotionAnimDataAsset> LocomotionAnimData;

	// Hit reaction animations (StaggerBS, KnockdownMontage, GetUpMontage,
	// GroggyStartMontage, GroggyEndMontage).

	UPROPERTY(EditDefaultsOnly, Category = "RV|AnimationAsset")
	TObjectPtr<URVHitReactionAnimDataAsset> HitReactionAnimData;

};