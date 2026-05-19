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
	Phase3 UMETA(DisplayName = "Phase 3"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRVOnBossPhaseChanged, ERVBossPhase, NewPhase);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRVOnBossGroggyStarted);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRVOnBossGroggyEnded);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRVOnSoulSiphonCompleted, bool, bHealed);

UCLASS()
class REVENANT_API ARVSevarogCharacter : public ARVCharacterBase
{
	GENERATED_BODY()

public:
	ARVSevarogCharacter();

	// --- StateTree task interface --------------------------------------------

	// Selects a random pattern from the current phase and starts the combo chain.
	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	void ExecutePhaseAttack();

	// Plays the rush arrival slam montage (SevarogData->RushAttackMontage).
	// Phase-independent — same attack fires regardless of current phase.
	// Called by STT_Rush after EndRush() on arrival.
	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	void ExecuteRushAttack();

	// Channel Soul_Siphon — heals if not interrupted.
	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	void ExecuteSoulSiphon();

	// Summon ground field via AnimNotify_SpawnGroundField mid-montage.
	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	void ExecuteSubjugation();

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	void StartGroggy();

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	void EndGroggy();

	// --- Rush ----------------------------------------------------------------

	// Sets bIsRushing = true and overrides MaxWalkSpeed with RushSpeed.
	// AnimInstance switches to RunLocomotionBS while this is true.
	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	void StartRush();

	// Restores MaxWalkSpeed to the DT_EnemyStats value and clears bIsRushing.
	// Records LastRushEndTime for cooldown tracking.
	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	void EndRush();

	// --- Backpedal -----------------------------------------------------------

	// Disables orient-to-movement so the boss can face the player while retreating.
	// Caller (STT_Backpedal) is responsible for focus and MoveToLocation calls.
	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	void StartBackpedal();

	// Restores orient-to-movement and clears controller-desired-rotation.
	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	void EndBackpedal();

	// --- State queries -------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	ERVBossPhase GetCurrentPhase() const { return CurrentPhase; }

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	bool IsGroggy() const { return bIsGroggy; }

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	bool IsAttacking() const;

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	bool IsRushing() const { return bIsRushing; }

	// --- Cooldown queries ----------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	bool IsAttackOnCooldown() const;

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	bool IsRushOnCooldown() const;

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	bool IsSoulSiphonOnCooldown() const;

	// Called by URVSevarogAnimInstance to cache BlendSpaces at init.
	const URVSevarogDataAsset* GetSevarogData() const { return SevarogData; }

	// --- Delegates -----------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "RV|Boss")
	FRVOnBossPhaseChanged OnBossPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "RV|Boss")
	FRVOnBossGroggyStarted OnBossGroggyStarted;

	UPROPERTY(BlueprintAssignable, Category = "RV|Boss")
	FRVOnBossGroggyEnded OnBossGroggyEnded;

	// Fired when the Soul_Siphon montage finishes (interrupted or not).
	// bHealed == true only when the montage completed uninterrupted.
	UPROPERTY(BlueprintAssignable, Category = "RV|Boss")
	FRVOnSoulSiphonCompleted OnSoulSiphonCompleted;

protected:
	virtual void BeginPlay() override;

	virtual URVHitReactionAnimDataAsset* GetHitReactionAnimData() const override;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss")
	TObjectPtr<URVSevarogDataAsset> SevarogData;

private:
	ERVBossPhase CurrentPhase = ERVBossPhase::Phase1;
	bool bIsGroggy             = false;
	bool bIsRushing            = false;
	int32 CurrentPoiseDepletionCount = 0;

	// Cached from DT_EnemyStats at BeginPlay — restored by EndRush().
	float NormalWalkSpeed = 400.f;

	// Timestamps for cooldown tracking. Updated in montage blend-out callbacks.
	float LastAttackEndTime    = -999.f;
	float LastRushEndTime      = -999.f;
	float LastSoulSiphonTime   = -999.f;

	FTimerHandle GroggyTimerHandle;

	// Active combo chain state.
	// Populated by ExecutePhaseAttack/ExecuteRushAttack; advanced by OnAttackMontageBlendingOut.
	TArray<TObjectPtr<UAnimMontage>> ActiveComboMontages;
	int32 ActiveComboIndex = 0;

	void StartComboChain(const TArray<TObjectPtr<UAnimMontage>>& InMontages);
	void PlayComboMontageAt(int32 InIndex);
	void OnAttackMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);

	void SetBossPhase(ERVBossPhase InNewPhase);

	UFUNCTION()
	void CheckPhaseTransition(float InNewHealth, float InDelta);

	UFUNCTION()
	void OnPoiseDepleted();

	// Called by AnimNotify_SpawnGroundField at the key frame of SubjugationMontage.
	void SpawnGroundField();
};