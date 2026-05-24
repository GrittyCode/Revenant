#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Data/RVEnemyStatRow.h"
#include "RVSevarogDataAsset.generated.h"

class UBlendSpace;
class URVHitReactionAnimDataAsset;
class UAnimMontage;
class UParticleSystem;

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

    // Max distance at which the BT will trigger this attack.
    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0")) float EngagementRange  = 200.f;
    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0")) float Cooldown         = 20.f;
    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0")) float HitDamage        = 40.f;
    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0")) float HitPoiseDamage   = 30.f;
    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0")) float HitRadius        = 300.f;
    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0")) float HitForwardOffset = 10.f;

    UPROPERTY(EditDefaultsOnly) TObjectPtr<UParticleSystem> CastFX;       // P_SiphonCasting
    UPROPERTY(EditDefaultsOnly) TObjectPtr<UParticleSystem> BodySwirlsFX; // P_SoulSwirlsBody
    UPROPERTY(EditDefaultsOnly) TObjectPtr<UParticleSystem> CastTrailsFX; // P_SoulCastTrails
    UPROPERTY(EditDefaultsOnly) TObjectPtr<UParticleSystem> HandFX;       // P_GhostHand
    UPROPERTY(EditDefaultsOnly) TObjectPtr<UParticleSystem> ImpactFX;     // P_SiphonImpact
};

USTRUCT(BlueprintType)
struct FRVSubjugationData
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly) TObjectPtr<UAnimMontage> Montage;

    // Max distance at which the BT will trigger this attack.
    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0")) float EngagementRange  = 400.f;
    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0")) float Cooldown         = 30.f;
    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0")) float BlastDamage      = 60.f;
    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0")) float BlastPoiseDamage = 40.f;
    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0")) float SwirlSpreadRadius = 400.f;
    UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0")) float SwirlDamageRadius = 150.f;

    UPROPERTY(EditDefaultsOnly) TObjectPtr<UParticleSystem> CastFX;   // P_Sub_Cast
    UPROPERTY(EditDefaultsOnly) TObjectPtr<UParticleSystem> BlastFX;  // P_Sevarog_Subjugate_Blast
    UPROPERTY(EditDefaultsOnly) TObjectPtr<UParticleSystem> SwirlsFX; // P_SubjugateSwirls
};

UCLASS()
class REVENANT_API URVSevarogDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    //--- Identity ------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "Name")
    FText BossName;

    //--- Stats ---------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
    FDataTableRowHandle EnemyStatRowHandle;

    FORCEINLINE const FRVEnemyStatRow* GetEnemyStatRow() const
    {
        if (EnemyStatRowHandle.IsNull()) { return nullptr; }
        return EnemyStatRowHandle.GetRow<FRVEnemyStatRow>(TEXT("URVSevarogDataAsset::GetEnemyStatRow"));
    }

    //--- Phase ---------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "Phase",
        meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Phase2Threshold = 0.50f;

    UPROPERTY(EditDefaultsOnly, Category = "Attacks")
    FRVBossPhaseAttacks Phase1Attacks;

    UPROPERTY(EditDefaultsOnly, Category = "Attacks")
    FRVBossPhaseAttacks Phase2Attacks;

    //--- Combat --------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat",
        meta = (ClampMin = "0.0"))
    float MeleeEngagementRange = 250.f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat",
        meta = (ClampMin = "0.0", ClampMax = "180.0"))
    float MaxComboTurnDegrees = 60.f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float GroggyDuration = 4.f;

    //--- Movement ------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "Movement",
        meta = (ClampMin = "0.0"))
    float ArrivalRange = 200.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement",
        meta = (ClampMin = "0.0"))
    float RushTriggerRadius = 700.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement",
        meta = (ClampMin = "0.0"))
    float RushSpeed = 700.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement",
        meta = (ClampMin = "0.0"))
    float RushCooldown = 5.f;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    TObjectPtr<UAnimMontage> RushAttackMontage;

    //--- Animation -----------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimationAsset")
    TObjectPtr<UBlendSpace> LocomotionBS;

    UPROPERTY(EditDefaultsOnly, Category = "AnimationAsset")
    TObjectPtr<URVHitReactionAnimDataAsset> HitReactionAnimData;

    //--- Special Attacks -----------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "SoulSiphon")
    FRVSoulSiphonData SoulSiphon;

    UPROPERTY(EditDefaultsOnly, Category = "Subjugation")
    FRVSubjugationData Subjugation;
};