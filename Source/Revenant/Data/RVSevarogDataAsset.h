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

	// Higher weight = more frequent selection. Minimum effective value is 1.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "1"))
	int32 Weight = 1;
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
	float Phase2Threshold = 0.50f;

	//--- Phase Attack Sets ---------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Attacks")
	FRVBossPhaseAttacks Phase1Attacks;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Attacks")
	FRVBossPhaseAttacks Phase2Attacks;

	//--- Attack Range --------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Boss|Combat",
		meta = (ClampMin = "0.0"))
	float AttackRadius = 300.f;

	//--- Groggy --------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Groggy")
	int32 GroggyPoiseDepletionCount = 3;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Groggy")
	float GroggyDuration = 4.f;

	//--- Rush ----------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Boss|Rush",
		meta = (ClampMin = "0.0"))
	float RushTriggerRadius = 700.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Boss|Rush",
		meta = (ClampMin = "0.0"))
	float RushSpeed = 700.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Boss|Rush",
		meta = (ClampMin = "0.0"))
	float RushCooldown = 5.f;

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

	// Sphere radius for the ghost-hand strike overlap.
	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|SoulSiphon",
		meta = (ClampMin = "0.0"))
	float SoulSiphonHitRadius = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|SoulSiphon",
		meta = (ClampMin = "0.0"))
	float SoulSiphonCooldown = 20.f;

	//--- Subjugation ---------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Subjugation")
	TObjectPtr<UAnimMontage> SubjugationMontage;

	// Blast sphere radius. Damage fires from AnimNotify_SubjugationBlast mid-montage.
	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Subjugation",
		meta = (ClampMin = "0.0"))
	float SubjugationBlastRadius = 400.f;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Subjugation",
		meta = (ClampMin = "0.0"))
	float SubjugationBlastDamage = 60.f;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Subjugation",
		meta = (ClampMin = "0.0"))
	float SubjugationBlastPoiseDamage = 40.f;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Subjugation",
		meta = (ClampMin = "0.0"))
	float SubjugationCooldown = 30.f;
};