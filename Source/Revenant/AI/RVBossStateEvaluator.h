// Source/Revenant/AI/RVBossStateEvaluator.h
#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "RVBossStateEvaluator.generated.h"

class ARVAIController;

USTRUCT()
struct FRVBossStateEvaluatorInstanceData
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<ARVSevarogCharacter> BossCharacter = nullptr;

	UPROPERTY()
	TObjectPtr<ARVAIController> CachedAIController = nullptr;

	// --- Outputs read by StateTree conditions --------------------------------

	UPROPERTY(EditAnywhere, Category = Output)
	TObjectPtr<APawn> PlayerPawn = nullptr;

	UPROPERTY(EditAnywhere, Category = Output)
	float DistToPlayer = 0.f;

	UPROPERTY(EditAnywhere, Category = Output)
	bool bIsGroggy = false;

	UPROPERTY(EditAnywhere, Category = Output)
	bool bIsAttacking = false;

	UPROPERTY(EditAnywhere, Category = Output)
	ERVBossPhase CurrentPhase = ERVBossPhase::Phase1;

	UPROPERTY(EditAnywhere, Category = Output)
	float BossHealthRatio = 1.f;

	// True while boss is in the gap-close rush approach.
	UPROPERTY(EditAnywhere, Category = Output)
	bool bIsRushing = false;

	// True for RushCooldown seconds after the previous rush ended.
	// STT_Rush entry condition: DistToPlayer > RushTriggerRange AND bIsRushOnCooldown == false.
	UPROPERTY(EditAnywhere, Category = Output)
	bool bIsRushOnCooldown = false;

	// True for AttackCooldownDuration seconds after any attack montage ends.
	// Drives the Backpedal state — boss retreats until this clears.
	UPROPERTY(EditAnywhere, Category = Output)
	bool bIsAttackOnCooldown = false;

	// True for SoulSiphonCooldown seconds after Soul_Siphon finishes (interrupted or not).
	// Prevents immediate Soul_Siphon re-entry after interruption.
	UPROPERTY(EditAnywhere, Category = Output)
	bool bIsSoulSiphonOnCooldown = false;
};

USTRUCT(meta = (DisplayName = "RV Boss State Evaluator"))
struct FRVBossStateEvaluator : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRVBossStateEvaluatorInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};