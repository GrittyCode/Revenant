#pragma once

#include "CoreMinimal.h"
#include "Character/Base/RVCharacterBase.h"
#include "RVSevarogCharacter.generated.h"

class URVSevarogDataAsset;
class UNiagaraComponent;
class UParticleSystem;
class USoundBase;

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

    //--- BT task interface ---------------------------------------------------

    bool ExecutePhaseAttack();
    bool ExecuteRushAttack();
    bool ExecuteSoulSiphon();
    bool ExecuteSubjugation();

    void ForceEndCurrentAction();

    void StartGroggy();
    void EndGroggy();

    void StartRush();
    void EndRush();

    void TryChainCombo();

    void InitSubjugationLocations(UParticleSystem* InCastFX);

    // InDamageDelay: seconds after SwirlsFX spawns before damage + SFX fire.
    void SpawnSubjugationBlast(UParticleSystem* InSwirlsFX, USoundBase* InSwirlsSFX);

    void ExecuteSoulSiphonHit();

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
    virtual void InitStats() override;
    virtual URVHitReactionAnimDataAsset* GetHitReactionAnimData() const override;

    virtual void ActivateWeaponTrail()   override;
    virtual void DeactivateWeaponTrail() override;

private:
    UPROPERTY()
    TObjectPtr<URVSevarogDataAsset> SevarogData;

    UPROPERTY()
    TObjectPtr<UNiagaraComponent> MeleeTrailNC;

    ERVBossPhase CurrentPhase     = ERVBossPhase::Phase1;
    bool         bIsGroggy        = false;
    bool         bIsRushing       = false;
    bool         bIsComboChaining = false;

    float NormalWalkSpeed = 400.f;

    //--- Combo chain ---------------------------------------------------------

    TArray<TObjectPtr<UAnimMontage>> ActiveComboMontages;
    int32 ActiveComboIndex = 0;

    void StartComboChain(const TArray<TObjectPtr<UAnimMontage>>& InMontages);
    void PlayComboMontageAt(int32 InIndex);
    bool PlaySingleShotAction(UAnimMontage* InMontage);

    static int32 SelectWeightedPattern(const TArray<struct FRVBossAttackPattern>& InPatterns);

    //--- Damage helpers ------------------------------------------------------

    void ApplyRadialDamageAt(const FVector& InLocation, float InRadius,
        float InDamage, float InPoiseDamage,
        UParticleSystem* InHitFX = nullptr,
        const FVector& InOverrideDirection = FVector::ZeroVector,
        bool bUseCapsule = false);

    //--- Subjugation ---------------------------------------------------------

    // Positions set by InitSubjugationLocations, read by SpawnSubjugationBlast
    // and ApplySubjugationDamage. Cleared by ForceEndCurrentAction.
    TArray<FVector> PendingSubjugationLocations;

    // SFX stored from SpawnSubjugationBlast, played in ApplySubjugationDamage.
    UPROPERTY()
    TObjectPtr<USoundBase> PendingSwirlsSFX;

    // Fires InDamageDelay seconds after SpawnSubjugationBlast.
    FTimerHandle SubjugationDamageTimerHandle;

    static TArray<FVector> GenerateSwirlLocations(const FVector& InOrigin,
        float InSpreadRadius, float InMinSeparation, int32 InCount);

    // Timer callback: applies damage + SFX at all pending swirl locations.
    UFUNCTION()
    void ApplySubjugationDamage();

    //--- VFX helpers ---------------------------------------------------------

    void SpawnFXAtLocation(UParticleSystem* InFX, const FVector& InLocation,
        const FRotator& InRotation = FRotator::ZeroRotator,
        const FVector& InScale = FVector::OneVector) const;

    //--- Internal helpers ----------------------------------------------------

    APawn* ResolvePlayerPawn() const;
    void RotateToFacePlayer(const APawn* InPlayer);
    void SetBossPhase(ERVBossPhase InNewPhase);

    void OnDeathMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);

    //--- Dissolve ------------------------------------------------------------

    void StartDissolve();
    void TickDissolve();

    UPROPERTY()
    TArray<TObjectPtr<UMaterialInstanceDynamic>> DissolveMIDs;

    FTimerHandle DissolveTimerHandle;
    float        DissolveElapsed  = 0.f;
    float        DissolveDuration = 2.f;

    UFUNCTION() void OnGroggySequenceCompleted();
    UFUNCTION() void OnAttackMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);
    UFUNCTION() void OnSingleShotActionBlendingOut(UAnimMontage* InMontage, bool bInterrupted);
    UFUNCTION() void CheckPhaseTransition(float InNewHealth, float InDelta);
    UFUNCTION() void OnPoiseDepleted();
};