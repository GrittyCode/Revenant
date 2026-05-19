#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "RVSevarogDataAsset.generated.h"

class URVLocomotionAnimDataAsset;
class URVHitReactionAnimDataAsset;
class UAnimMontage;
struct FRVEnemyStatRow;

USTRUCT(BlueprintType)
struct FRVBossAttackPattern
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TObjectPtr<UAnimMontage>> ComboMontages;
};

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

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Attacks")
	FRVBossPhaseAttacks Phase1Attacks;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Attacks")
	FRVBossPhaseAttacks Phase2Attacks;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Attacks")
	FRVBossPhaseAttacks Phase3Attacks;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Attacks")
	FRVBossPhaseAttacks RushFollowupAttacks;

	//--- Attack Range --------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Boss|Combat",
		meta = (ClampMin = "0.0"))
	float AttackRange = 300.f;

	//--- Attack Cooldown (페이즈별) -------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Boss|Combat",
		meta = (ClampMin = "0.0"))
	float Phase1AttackCooldown = 2.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Boss|Combat",
		meta = (ClampMin = "0.0"))
	float Phase2AttackCooldown = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Boss|Combat",
		meta = (ClampMin = "0.0"))
	float Phase3AttackCooldown = 0.8f;

	//--- Groggy --------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Groggy")
	int32 GroggyPoiseDepletionCount = 3;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Groggy")
	float GroggyDuration = 4.f;

	//--- Rush ----------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Boss|Rush",
		meta = (ClampMin = "0.0"))
	float RushTriggerRange = 700.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Boss|Rush",
		meta = (ClampMin = "0.0"))
	float RushArrivalRange = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Boss|Rush",
		meta = (ClampMin = "0.0"))
	float RushSpeed = 700.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Boss|Rush",
		meta = (ClampMin = "0.0"))
	float RushCooldown = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Rush")
	TObjectPtr<UAnimMontage> RushAttackMontage;

	//--- Animation Assets ----------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|AnimationAsset")
	TObjectPtr<URVLocomotionAnimDataAsset> LocomotionAnimData;

	UPROPERTY(EditDefaultsOnly, Category = "RV|AnimationAsset")
	TObjectPtr<URVHitReactionAnimDataAsset> HitReactionAnimData;

	//--- Soul Siphon ---------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|SoulSiphon")
	TObjectPtr<UAnimMontage> SoulSiphonMontage;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|SoulSiphon",
		meta = (ClampMin = "0.0"))
	float SoulSiphonHealAmount = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|SoulSiphon",
		meta = (ClampMin = "0.0"))
	float SoulSiphonCooldown = 20.f;

	//--- Subjugation ---------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Subjugation")
	TObjectPtr<UAnimMontage> SubjugationMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Boss|Subjugation",
		meta = (ClampMin = "0.0"))
	float SubjugationCooldown = 30.f;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Subjugation",
		meta = (ClampMin = "0.0"))
	float GroundFieldDuration = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Subjugation",
		meta = (ClampMin = "0.0"))
	float GroundFieldDamagePerTick = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Subjugation",
		meta = (ClampMin = "0.0"))
	float GroundFieldPoiseDamage = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Subjugation",
		meta = (ClampMin = "0.0"))
	float GroundFieldTickInterval = 0.5f;
};