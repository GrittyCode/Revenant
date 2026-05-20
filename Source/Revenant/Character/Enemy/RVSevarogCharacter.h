#pragma once

#include "CoreMinimal.h"
#include "Character/Base/RVCharacterBase.h"
#include "RVSevarogCharacter.generated.h"

class URVSevarogDataAsset;

UENUM(BlueprintType)
enum class ERVBossPhase : uint8
{
	Phase1 UMETA(DisplayName = "Phase 1"),
	Phase2 UMETA(DisplayName = "Phase 2"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRVOnBossPhaseChanged, ERVBossPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRVOnBossGroggyStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRVOnBossGroggyEnded);

// Non-dynamic — supports AddWeakLambda binding from BT tasks.
DECLARE_MULTICAST_DELEGATE(FRVOnBossAttackFinished);

UCLASS()
class REVENANT_API ARVSevarogCharacter : public ARVCharacterBase
{
	GENERATED_BODY()

public:
	ARVSevarogCharacter();

	void RotateToFacePlayer();

	// --- BT task interface ---------------------------------------------------

	/** Returns false if preconditions fail (groggy, already attacking, empty pattern set). */
	bool ExecutePhaseAttack();

	/** Returns false if preconditions fail or RushAttackMontage is unassigned. */
	bool ExecuteRushAttack();

	/** Returns false if preconditions fail or SoulSiphonMontage is unassigned. */
	bool ExecuteSoulSiphon();

	/** Returns false if preconditions fail or SubjugationMontage is unassigned. */
	bool ExecuteSubjugation();

	void ForceEndCurrentAction();

	void StartGroggy();
	void EndGroggy();

	// --- Rush ----------------------------------------------------------------

	void StartRush();
	void EndRush();

	// --- Combo chain (called from AnimNotify_BossComboChain) -----------------

	/**
	 * Called by AnimNotify_BossComboChain placed at the start of the Return2Idle section.
	 * If more combo hits are queued, stops the current montage and plays the next one.
	 * If this is the last hit, does nothing — Return2Idle plays and BlendingOut cleans up.
	 */
	void TryChainCombo();

	// --- Subjugation blast (called from AnimNotify_SubjugationBlast) ---------

	void SpawnSubjugationBlast();

	// --- State queries -------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	ERVBossPhase GetCurrentPhase() const { return CurrentPhase; }

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	bool IsGroggy() const { return bIsGroggy; }

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	bool IsAttacking() const;

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	bool IsRushing() const { return bIsRushing; }

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	const URVSevarogDataAsset* GetSevarogData() const { return SevarogData; }

	// --- Delegates -----------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "RV|Boss")
	FRVOnBossPhaseChanged OnBossPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "RV|Boss")
	FRVOnBossGroggyStarted OnBossGroggyStarted;

	UPROPERTY(BlueprintAssignable, Category = "RV|Boss")
	FRVOnBossGroggyEnded OnBossGroggyEnded;

	// Fired when any attack action finishes naturally (phase attack, soul siphon, subjugation).
	// BT tasks bind via AddWeakLambda and call FinishLatentTask from here instead of polling TickTask.
	FRVOnBossAttackFinished OnAttackFinished;

protected:
	virtual void BeginPlay() override;

	virtual URVHitReactionAnimDataAsset* GetHitReactionAnimData() const override;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss")
	TObjectPtr<URVSevarogDataAsset> SevarogData;

private:
	ERVBossPhase CurrentPhase               = ERVBossPhase::Phase1;
	bool         bIsGroggy                  = false;
	bool         bIsRushing                 = false;
	bool         bIsComboChaining           = false;
	int32        CurrentPoiseDepletionCount = 0;

	float NormalWalkSpeed = 400.f;

	FTimerHandle GroggyTimerHandle;

	TArray<TObjectPtr<UAnimMontage>> ActiveComboMontages;
	int32 ActiveComboIndex = 0;

	void StartComboChain(const TArray<TObjectPtr<UAnimMontage>>& InMontages);
	void PlayComboMontageAt(int32 InIndex);

	int32 SelectWeightedPattern(const TArray<struct FRVBossAttackPattern>& InPatterns) const;

	void SetBossPhase(ERVBossPhase InNewPhase);

	UFUNCTION() void OnAttackMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);
	UFUNCTION() void OnSoulSiphonBlendingOut(UAnimMontage* InMontage, bool bInterrupted);
	UFUNCTION() void OnSubjugationMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);

	UFUNCTION() void OnStunStartMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);
	UFUNCTION() void OnStunEndMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);

	UFUNCTION() void CheckPhaseTransition(float InNewHealth, float InDelta);
	UFUNCTION() void OnPoiseDepleted();
};