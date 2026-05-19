#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "RVSevarogTasks.generated.h"

class ARVSevarogCharacter;
class ARVAIController;

// ---------------------------------------------------------------------------
// FRVChaseTaskInstanceData
// ---------------------------------------------------------------------------
USTRUCT()
struct FRVChaseTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<ARVSevarogCharacter> BossCharacter = nullptr;

	UPROPERTY()
	TObjectPtr<ARVAIController> CachedController = nullptr;
};

// ---------------------------------------------------------------------------
// FRVRushTaskInstanceData
// ---------------------------------------------------------------------------
USTRUCT()
struct FRVRushTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<ARVSevarogCharacter> BossCharacter = nullptr;

	UPROPERTY()
	TObjectPtr<ARVAIController> CachedController = nullptr;

	UPROPERTY()
	bool bAttackLaunched = false;
};

// ---------------------------------------------------------------------------
// FRVBossActionTaskInstanceData
// PhaseAttack / SoulSiphon / Subjugation / WaitGroggyEnd 공용.
// ---------------------------------------------------------------------------
USTRUCT()
struct FRVBossActionTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<ARVSevarogCharacter> BossCharacter = nullptr;
};

// =============================================================================
// FRVChaseTask
// =============================================================================
USTRUCT(meta = (DisplayName = "RV Chase"))
struct REVENANT_API FRVChaseTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRVChaseTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context,
		const float DeltaTime) const override;
};

// =============================================================================
// FRVRushTask
// =============================================================================
USTRUCT(meta = (DisplayName = "RV Rush"))
struct REVENANT_API FRVRushTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRVRushTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context,
		const float DeltaTime) const override;

	virtual void ExitState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};

// =============================================================================
// FRVPhaseAttackTask
// =============================================================================
USTRUCT(meta = (DisplayName = "RV Phase Attack"))
struct REVENANT_API FRVPhaseAttackTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRVBossActionTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context,
		const float DeltaTime) const override;

	virtual void ExitState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};

// =============================================================================
// FRVSoulSiphonTask
// =============================================================================
USTRUCT(meta = (DisplayName = "RV Soul Siphon"))
struct REVENANT_API FRVSoulSiphonTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRVBossActionTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context,
		const float DeltaTime) const override;

	virtual void ExitState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};

// =============================================================================
// FRVSubjugationTask
// =============================================================================
USTRUCT(meta = (DisplayName = "RV Subjugation"))
struct REVENANT_API FRVSubjugationTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRVBossActionTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context,
		const float DeltaTime) const override;

	virtual void ExitState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};

// =============================================================================
// FRVWaitGroggyEndTask
// =============================================================================
USTRUCT(meta = (DisplayName = "RV Wait Groggy End"))
struct REVENANT_API FRVWaitGroggyEndTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRVBossActionTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context,
		const float DeltaTime) const override;
};