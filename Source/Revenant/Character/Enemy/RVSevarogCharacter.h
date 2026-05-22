// Source/Revenant/Character/Enemy/RVSevarogCharacter.h
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
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRVOnBossDefeated);
DECLARE_MULTICAST_DELEGATE(FRVOnBossGroggyStarted);
DECLARE_MULTICAST_DELEGATE(FRVOnBossGroggyEnded);
DECLARE_MULTICAST_DELEGATE(FRVOnBossAttackFinished);

UCLASS()
class REVENANT_API ARVSevarogCharacter : public ARVCharacterBase
{
    GENERATED_BODY()

public:
    ARVSevarogCharacter();

    void RotateToFacePlayer(const APawn* InPlayer);

    //--- BT task interface ---------------------------------------------------

    bool ExecutePhaseAttack();
    bool ExecuteRushAttack();
    bool ExecuteSoulSiphon();
    bool ExecuteSubjugation();

    void ForceEndCurrentAction();

    void StartGroggy();
    void EndGroggy();

    //--- Rush ----------------------------------------------------------------

    void StartRush();
    void EndRush();

    //--- Combo chain ---------------------------------------------------------

    void TryChainCombo();

    //--- Subjugation ---------------------------------------------------------

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

    UPROPERTY(BlueprintAssignable, Category = "RV|Boss")
    FRVOnBossDefeated OnBossDefeated;

    FRVOnBossGroggyStarted  OnBossGroggyStarted;
    FRVOnBossGroggyEnded    OnBossGroggyEnded;
    FRVOnBossAttackFinished OnAttackFinished;

protected:
    virtual void BeginPlay() override;
    virtual void OnDeath() override;

    virtual URVHitReactionAnimDataAsset* GetHitReactionAnimData() const override;

    UPROPERTY(EditDefaultsOnly, Category = "RV|Boss")
    TObjectPtr<URVSevarogDataAsset> SevarogData;

private:
    ERVBossPhase CurrentPhase     = ERVBossPhase::Phase1;
    bool         bIsGroggy        = false;
    bool         bIsRushing       = false;
    bool         bIsComboChaining = false;

    float NormalWalkSpeed = 400.f;

    TArray<TObjectPtr<UAnimMontage>> ActiveComboMontages;
    int32 ActiveComboIndex = 0;

    void StartComboChain(const TArray<TObjectPtr<UAnimMontage>>& InMontages);
    void PlayComboMontageAt(int32 InIndex);

    int32 SelectWeightedPattern(const TArray<struct FRVBossAttackPattern>& InPatterns) const;

    void SetBossPhase(ERVBossPhase InNewPhase);

    bool PlaySingleShotAction(UAnimMontage* InMontage);

    void OnDeathMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);

    UFUNCTION() void OnGroggySequenceCompleted();
    UFUNCTION() void OnAttackMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);
    UFUNCTION() void OnSingleShotActionBlendingOut(UAnimMontage* InMontage, bool bInterrupted);
    UFUNCTION() void CheckPhaseTransition(float InNewHealth, float InDelta);
    UFUNCTION() void OnPoiseDepleted();
};