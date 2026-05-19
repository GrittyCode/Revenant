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

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	void ExecutePhaseAttack();

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	void ExecuteRushAttack();

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	void ExecuteSoulSiphon();

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	void ExecuteSubjugation();

	void ForceEndCurrentAction();

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	void StartGroggy();

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	void EndGroggy();

	// --- Rush ----------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	void StartRush();

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	void EndRush();

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
	bool IsJustRushed() const { return bJustRushed; }

	// --- Cooldown queries ----------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	bool IsAttackOnCooldown() const;

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	bool IsRushOnCooldown() const;

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	bool IsSoulSiphonOnCooldown() const;

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	bool IsSubjugationOnCooldown() const;

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	const URVSevarogDataAsset* GetSevarogData() const { return SevarogData; }

	// --- Delegates -----------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "RV|Boss")
	FRVOnBossPhaseChanged OnBossPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "RV|Boss")
	FRVOnBossGroggyStarted OnBossGroggyStarted;

	UPROPERTY(BlueprintAssignable, Category = "RV|Boss")
	FRVOnBossGroggyEnded OnBossGroggyEnded;

	UPROPERTY(BlueprintAssignable, Category = "RV|Boss")
	FRVOnSoulSiphonCompleted OnSoulSiphonCompleted;

protected:
	virtual void BeginPlay() override;

	virtual URVHitReactionAnimDataAsset* GetHitReactionAnimData() const override;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss")
	TObjectPtr<URVSevarogDataAsset> SevarogData;

private:
	ERVBossPhase CurrentPhase               = ERVBossPhase::Phase1;
	bool         bIsGroggy                  = false;
	bool         bIsRushing                 = false;
	bool         bJustRushed                = false;
	int32        CurrentPoiseDepletionCount = 0;

	float NormalWalkSpeed     = 400.f;

	float LastAttackEndTime   = -BIG_NUMBER;
	float LastRushEndTime     = -BIG_NUMBER;
	float LastSoulSiphonTime  = -BIG_NUMBER;
	float LastSubjugationTime = -BIG_NUMBER;

	FTimerHandle GroggyTimerHandle;

	TArray<TObjectPtr<UAnimMontage>> ActiveComboMontages;
	int32 ActiveComboIndex = 0;

	void StartComboChain(const TArray<TObjectPtr<UAnimMontage>>& InMontages);
	void PlayComboMontageAt(int32 InIndex);
	float GetAttackCooldown() const;
	void SetBossPhase(ERVBossPhase InNewPhase);

	UFUNCTION()
	void OnAttackMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);

	UFUNCTION()
	void OnSoulSiphonMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);

	UFUNCTION()
	void OnSubjugationMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);

	UFUNCTION()
	void CheckPhaseTransition(float InNewHealth, float InDelta);

	UFUNCTION()
	void OnPoiseDepleted();

	void SpawnGroundField();
};