// Source/Revenant/Character/Enemy/RVSevarogCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "Character/Base/RVCharacterBase.h"
#include "Component/RVCombatStateComponent.h"
#include "RVSevarogCharacter.generated.h"

class URVSevarogDataAsset;

UENUM(BlueprintType)
enum class ERVBossPhase : uint8
{
    Phase1 UMETA(DisplayName = "Phase 1"),
    Phase2 UMETA(DisplayName = "Phase 2"),
};

// Dynamic — BP / UI can subscribe via OnBossPhaseChanged.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRVOnBossPhaseChanged, ERVBossPhase, NewPhase);

// Dynamic — Win UI subscribes in Blueprint.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRVOnBossDefeated);

// Non-dynamic — BT tasks bind via AddWeakLambda; no BP graph subscription needed.
DECLARE_MULTICAST_DELEGATE(FRVOnBossGroggyStarted);
DECLARE_MULTICAST_DELEGATE(FRVOnBossGroggyEnded);

// Non-dynamic — BT tasks bind via AddWeakLambda.
DECLARE_MULTICAST_DELEGATE(FRVOnBossAttackFinished);

UCLASS()
class REVENANT_API ARVSevarogCharacter : public ARVCharacterBase
{
    GENERATED_BODY()

public:
    ARVSevarogCharacter();

    /** Rotates the boss to face InPlayer, clamped to MaxComboTurnDegrees. */
    void RotateToFacePlayer(const APawn* InPlayer);

    //--- BT task interface ---------------------------------------------------

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

    //--- Rush ----------------------------------------------------------------

    void StartRush();
    void EndRush();

    //--- Combo chain (called from AnimNotify_BossComboChain) -----------------

    /**
     * Called by AnimNotify_BossComboChain placed at the start of the Return2Idle section.
     * If more combo hits are queued, stops the current montage and plays the next one.
     * If this is the last hit, does nothing — Return2Idle plays and BlendingOut cleans up.
     */
    void TryChainCombo();

    //--- Subjugation blast (called from AnimNotify_SubjugationBlast) ---------

    void SpawnSubjugationBlast();

    //--- State queries -------------------------------------------------------

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

    //--- Delegates -----------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "RV|Boss")
    FRVOnBossPhaseChanged OnBossPhaseChanged;

    // Broadcast when HP reaches 0. Win UI subscribes here.
    UPROPERTY(BlueprintAssignable, Category = "RV|Boss")
    FRVOnBossDefeated OnBossDefeated;

    // Non-dynamic: BT tasks use AddWeakLambda.
    FRVOnBossGroggyStarted OnBossGroggyStarted;
    FRVOnBossGroggyEnded   OnBossGroggyEnded;

    // Fired when any attack action finishes naturally (phase attack, soul siphon, subjugation, rush).
    // Interrupted actions (bInterrupted=true in BlendingOut) do not fire this delegate.
    FRVOnBossAttackFinished OnAttackFinished;

protected:
    virtual void BeginPlay() override;
    virtual void OnDeath() override;

    virtual URVHitReactionAnimDataAsset* GetHitReactionAnimData() const override;

    UPROPERTY(EditDefaultsOnly, Category = "RV|Boss")
    TObjectPtr<URVSevarogDataAsset> SevarogData;

private:
    ERVBossPhase CurrentPhase              = ERVBossPhase::Phase1;
    bool         bIsGroggy                 = false;
    bool         bIsRushing                = false;
    bool         bIsComboChaining          = false;
    int32        CurrentPoiseDepletionCount = 0;

    float NormalWalkSpeed = 400.f;

    TArray<TObjectPtr<UAnimMontage>> ActiveComboMontages;
    int32 ActiveComboIndex = 0;

    void StartComboChain(const TArray<TObjectPtr<UAnimMontage>>& InMontages);
    void PlayComboMontageAt(int32 InIndex);

    int32 SelectWeightedPattern(const TArray<struct FRVBossAttackPattern>& InPatterns) const;

    void SetBossPhase(ERVBossPhase InNewPhase);

    /** Common play path for single-montage actions (SoulSiphon, Subjugation). */
    bool PlaySingleShotAction(UAnimMontage* InMontage);

    void OnDeathMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);

    // called when HitReactionComponent finishes the full groggy montage sequence
    UFUNCTION() void OnGroggySequenceCompleted();

    UFUNCTION() void OnAttackMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);
    UFUNCTION() void OnSingleShotActionBlendingOut(UAnimMontage* InMontage, bool bInterrupted);
    UFUNCTION() void CheckPhaseTransition(float InNewHealth, float InDelta);
    UFUNCTION() void OnPoiseDepleted();
};