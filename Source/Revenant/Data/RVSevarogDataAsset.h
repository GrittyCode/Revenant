#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Data/RVCharacterStatRow.h"
#include "RVSevarogDataAsset.generated.h"

class UBlendSpace;
class URVHitReactionAnimDataAsset;
class UAnimMontage;
class UParticleSystem;
class UNiagaraSystem;
class USoundBase;

USTRUCT(BlueprintType)
struct FRVBossAttackPattern
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TObjectPtr<UAnimMontage>> ComboMontages;

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

USTRUCT(BlueprintType)
struct FRVSoulSiphonData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly) TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0")) float EngagementRange  = 200.f;
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0")) float Cooldown         = 20.f;
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0")) float HitDamage        = 40.f;
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0")) float HitPoiseDamage   = 30.f;
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0")) float HitRadius        = 300.f;
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0")) float HitForwardOffset = 10.f;
	
	
	UPROPERTY(EditDefaultsOnly) TObjectPtr<UParticleSystem> ImpactFX;
};

USTRUCT(BlueprintType)
struct FRVSubjugationData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly) TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0")) float EngagementRange   = 400.f;
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0")) float Cooldown          = 30.f;
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0")) float BlastDamage       = 60.f;
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0")) float BlastPoiseDamage  = 40.f;
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0")) float SwirlSpreadRadius = 400.f;
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0")) float SwirlDamageRadius = 150.f;
	
};

UCLASS()
class REVENANT_API URVSevarogDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	//--- Stat ----------------------------------------------------------------

	// Points to a row in DT_BossStats.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (DisplayPriority = 0))
	FDataTableRowHandle StatRowHandle;

	FORCEINLINE const FRVCharacterStatRow* GetStatRow() const
	{
		if (StatRowHandle.IsNull()) { return nullptr; }
		return StatRowHandle.GetRow<FRVCharacterStatRow>(TEXT("URVSevarogDataAsset::GetBossStatRow"));
	}

	//--- Identity ------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "Name", meta = (DisplayPriority = 1))
	FText BossName;

	//--- Phase ---------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "Phase", meta = (DisplayPriority = 2,
		ClampMin = "0.0", ClampMax = "1.0"))
	float Phase2Threshold = 0.50f;

	UPROPERTY(EditDefaultsOnly, Category = "Attacks", meta = (DisplayPriority = 3))
	FRVBossPhaseAttacks Phase1Attacks;

	UPROPERTY(EditDefaultsOnly, Category = "Attacks", meta = (DisplayPriority = 3))
	FRVBossPhaseAttacks Phase2Attacks;

	//--- Combat --------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (DisplayPriority = 4,
		ClampMin = "0.0"))
	float MeleeEngagementRange = 250.f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat", meta = (DisplayPriority = 4,
		ClampMin = "0.0", ClampMax = "180.0"))
	float MaxComboTurnDegrees = 60.f;

	// Final damage = BaseDamage x DT_AttackStats.DamageMultiplier (via URVMontageStatData).
	UPROPERTY(EditDefaultsOnly, Category = "Combat", meta = (DisplayPriority = 4,
		ClampMin = "0.0"))
	float BaseDamage = 80.f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat", meta = (DisplayPriority = 4,
		ClampMin = "0.0"))
	float BasePoiseDamage = 40.f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat", meta = (DisplayPriority = 4,
		ClampMin = "0.0"))
	float AttackRadius = 55.f;

	// Cascade hit impact spawned at the struck actor's location on confirmed melee hits.
	UPROPERTY(EditDefaultsOnly, Category = "Combat", meta = (DisplayPriority = 4))
	TObjectPtr<UParticleSystem> MeleeHitImpact; // P_Sevarog_Melee_SucessfulImpact

	// Sound played at the struck actor's location on each confirmed melee hit.
	UPROPERTY(EditDefaultsOnly, Category = "Combat", meta = (DisplayPriority = 4))
	TObjectPtr<USoundBase> MeleeHitSFX;

	// Niagara trail attached to WeaponTip socket on melee swing.
	// Activated via AnimNotifyState_WeaponTrailFX. Same approach as player weapon trail.
	UPROPERTY(EditDefaultsOnly, Category = "Combat", meta = (DisplayPriority = 4))
	TObjectPtr<UNiagaraSystem> MeleeTrailEffect;

	// Ribbon width injected into User.Width parameter of the trail Niagara system.
	UPROPERTY(EditDefaultsOnly, Category = "Combat", meta = (DisplayPriority = 4, ClampMin = "0.0"))
	float MeleeTrailWidth = 15.f;

	//--- Movement ------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (DisplayPriority = 5,
		ClampMin = "0.0"))
	float ArrivalRange = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (DisplayPriority = 5,
		ClampMin = "0.0"))
	float RushTriggerRadius = 700.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (DisplayPriority = 5,
		ClampMin = "0.0"))
	float RushSpeed = 700.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (DisplayPriority = 5,
		ClampMin = "0.0"))
	float RushCooldown = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (DisplayPriority = 5))
	TObjectPtr<UAnimMontage> RushAttackMontage;

	//--- Animation -----------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (DisplayPriority = 6))
	TObjectPtr<UBlendSpace> LocomotionBS;

	// Duration (seconds) over which FadeOut 0→1 is driven on all mesh materials during death.
	UPROPERTY(EditDefaultsOnly, Category = "Animation", meta = (DisplayPriority = 6, ClampMin = "0.1"))
	float DissolveDuration = 2.f;

	UPROPERTY(EditDefaultsOnly, Category = "Animation", meta = (DisplayPriority = 6))
	TObjectPtr<URVHitReactionAnimDataAsset> HitReactionAnimData;

	//--- Special Attacks -----------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SoulSiphon", meta = (DisplayPriority = 7))
	FRVSoulSiphonData SoulSiphon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Subjugation", meta = (DisplayPriority = 8))
	FRVSubjugationData Subjugation;
};