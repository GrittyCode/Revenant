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

	UPROPERTY(EditAnywhere, Category = Output)
	bool bIsRushing = false;

	UPROPERTY(EditAnywhere, Category = Output)
	bool bIsRushOnCooldown = false;

	UPROPERTY(EditAnywhere, Category = Output)
	bool bIsAttackOnCooldown = false;

	UPROPERTY(EditAnywhere, Category = Output)
	bool bIsSoulSiphonOnCooldown = false;

	UPROPERTY(EditAnywhere, Category = Output)
	bool bIsSubjugationOnCooldown = false;

	UPROPERTY(EditAnywhere, Category = Output)
	bool bJustRushed = false;

	UPROPERTY(EditAnywhere, Category = Output)
	float AttackRange = 300.f;
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