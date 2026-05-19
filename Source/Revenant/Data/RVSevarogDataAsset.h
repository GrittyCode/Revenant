#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "RVSevarogDataAsset.generated.h"

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
class REVENANT_API URVSevarogDataAsset : public UPrimaryDataAsset
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

	//--- Attack Cooldown -----------------------------------------------------
	// Enforces the player-retaliation window after every attack ends.
	// Boss enters Backpedal state for this duration before the next AttackCycle.

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Combat", meta = (ClampMin = "0.0"))
	float AttackCooldownDuration = 2.5f;

	//--- Groggy --------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Groggy")
	int32 GroggyPoiseDepletionCount = 3;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Groggy")
	float GroggyDuration = 4.f;

	//--- Rush ----------------------------------------------------------------
	// Boss closes the gap at high speed, then executes a fixed Swing_C on arrival.
	// No dedicated rush montage — locomotion switches to RunLocomotionBS via bIsRushing.

	// DistToPlayer must exceed this for STT_Rush to become eligible.
	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Rush", meta = (ClampMin = "0.0"))
	float RushTriggerRange = 700.f;

	// DistToPlayer falls below this → EndRush() + ExecuteRushAttack().
	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Rush", meta = (ClampMin = "0.0"))
	float RushArrivalRange = 200.f;

	// MaxWalkSpeed override while rushing.
	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Rush", meta = (ClampMin = "0.0"))
	float RushSpeed = 700.f;

	// Seconds before another rush is allowed after the previous one ends.
	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Rush", meta = (ClampMin = "0.0"))
	float RushCooldown = 10.f;

	// Montage played immediately on rush arrival. Assign the single slam/downstrike montage here.
	// Phase-independent — same attack fires regardless of current phase.
	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Rush")
	TObjectPtr<UAnimMontage> RushAttackMontage;

	//--- Backpedal -----------------------------------------------------------
	// Boss faces the player and retreats along a NavMesh path after each attack.
	// bOrientRotationToMovement is disabled; controller focus holds the facing direction.

	// Target offset distance per MoveToLocation call (re-evaluated every 0.5s).
	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Backpedal", meta = (ClampMin = "0.0"))
	float BackpedalStepDistance = 300.f;

	// Backpedal ends when DistToPlayer exceeds this value.
	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Backpedal", meta = (ClampMin = "0.0"))
	float BackpedalMaxDist = 600.f;

	//--- Animation Assets ----------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|AnimationAsset")
	TObjectPtr<URVLocomotionAnimDataAsset> LocomotionAnimData;

	// Hit reaction animations (StaggerBS, KnockdownMontage, GetUpMontage,
	// GroggyStartMontage, GroggyEndMontage).
	UPROPERTY(EditDefaultsOnly, Category = "RV|AnimationAsset")
	TObjectPtr<URVHitReactionAnimDataAsset> HitReactionAnimData;

	//--- Soul Siphon ---------------------------------------------------------
	// Sevarog channels Soul_Siphon and heals if not interrupted by the player.
	// No hit judgment — pure self-heal on montage completion.

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|SoulSiphon")
	TObjectPtr<UAnimMontage> SoulSiphonMontage;

	// HP recovered when Soul_Siphon completes uninterrupted (flat amount).
	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|SoulSiphon", meta = (ClampMin = "0.0"))
	float SoulSiphonHealAmount = 200.f;

	// Seconds before Soul_Siphon can be used again (whether interrupted or not).
	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|SoulSiphon", meta = (ClampMin = "0.0"))
	float SoulSiphonCooldown = 20.f;

	//--- Subjugation ---------------------------------------------------------
	// Sevarog summons a ground field at his feet.
	// Field deals damage per tick to players who remain inside.

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Subjugation")
	TObjectPtr<UAnimMontage> SubjugationMontage;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Subjugation", meta = (ClampMin = "0.0"))
	float GroundFieldDuration = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Subjugation", meta = (ClampMin = "0.0"))
	float GroundFieldDamagePerTick = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Subjugation", meta = (ClampMin = "0.0"))
	float GroundFieldPoiseDamage = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Subjugation", meta = (ClampMin = "0.0"))
	float GroundFieldTickInterval = 0.5f;
};