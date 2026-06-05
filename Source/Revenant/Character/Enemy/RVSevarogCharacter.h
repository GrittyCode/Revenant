#pragma once

#include "CoreMinimal.h"
#include "Character/Base/RVCharacterBase.h"
#include "RVSevarogCharacter.generated.h"

class URVSevarogDataAsset;
class URVHitReactionComponent;
class UNiagaraComponent;
class UParticleSystem;
class USoundBase;

UENUM(BlueprintType)
enum class ERVBossPhase : uint8
{
    Phase1 UMETA(DisplayName = "Phase 1"),
    Phase2 UMETA(DisplayName = "Phase 2"),
};

DECLARE_MULTICAST_DELEGATE(FRVOnBossDefeated);
DECLARE_MULTICAST_DELEGATE(FRVOnBossGroggyStarted);
DECLARE_MULTICAST_DELEGATE(FRVOnBossAttackFinished);
DECLARE_MULTICAST_DELEGATE_OneParam(FRVOnBossPhaseChanged, ERVBossPhase);

UCLASS()
class REVENANT_API ARVSevarogCharacter : public ARVCharacterBase
{
    GENERATED_BODY()

public:
    ARVSevarogCharacter();

    virtual void ActivateWeaponTrail()   override;
    virtual void DeactivateWeaponTrail() override;

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
    void SpawnSubjugationBlast(UParticleSystem* InSwirlsFX, USoundBase* InSwirlsSFX);

    void ExecuteSoulSiphonHit();

    //--- State queries -------------------------------------------------------

    const URVSevarogDataAsset* GetSevarogData() const { return SevarogData; }
    ERVBossPhase GetCurrentPhase() const { return CurrentPhase; }
    bool IsGroggy()        const { return HasCombatState(ERVCombatState::Groggy); }
    bool IsAttacking()     const;
    bool IsRushing()       const { return bIsRushing; }
    bool IsInHitReaction() const;
    bool IsKnockedDown()   const;
    float GetStaggerDirectionForAnim() const;

    URVHitReactionComponent* GetHitReactionComponent() const { return HitReactionComponent; }

    //--- Delegates -----------------------------------------------------------

    FRVOnBossPhaseChanged   OnBossPhaseChanged;
    FRVOnBossDefeated       OnBossDefeated;
    FRVOnBossGroggyStarted  OnBossGroggyStarted;
    FRVOnBossAttackFinished OnAttackFinished;

protected:
    virtual void BeginPlay() override;
    virtual void OnDeath()   override;
    virtual void InitStats() override;
    virtual URVHitReactionAnimDataAsset* GetHitReactionAnimData() const override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "RV|Data")
    TObjectPtr<URVSevarogDataAsset> SevarogData;

    UPROPERTY(VisibleAnywhere, Category = "RV|Debug")
    TObjectPtr<UNiagaraComponent> MeleeTrailNC;

    ERVBossPhase CurrentPhase     = ERVBossPhase::Phase1;
    bool         bIsRushing       = false;
    bool         bIsComboChaining = false;

    float NormalWalkSpeed      = 400.f;
    float CachedGroggyDuration = 4.f;

    //--- Combo chain ---------------------------------------------------------

    TArray<TObjectPtr<UAnimMontage>> ActiveComboMontages;
    int32 ActiveComboIndex = 0;

    void StartComboChain(const TArray<TObjectPtr<UAnimMontage>>& InMontages);
    void PlayComboMontageAt(int32 InIndex);
    bool PlaySingleShotAction(UAnimMontage* InMontage);

    static int32 SelectWeightedPattern(const TArray<struct FRVBossAttackPattern>& InPatterns);

    //--- Hit FX --------------------------------------------------------------

    void OnHitConfirmedHandler(FVector ImpactLocation);

    //--- Damage helpers ------------------------------------------------------

    void ApplyDamageToOverlapResults(const TArray<FOverlapResult>& InOverlaps,
        float InDamage, float InPoiseDamage,
        UParticleSystem* InHitFX,
        const FVector& InOverrideDirection);

    void ApplySphereDamageAt(const FVector& InLocation, float InRadius,
        float InDamage, float InPoiseDamage,
        UParticleSystem* InHitFX = nullptr,
        const FVector& InOverrideDirection = FVector::ZeroVector);

    void ApplyForwardCapsuleDamageAt(const FVector& InLocation, float InRadius, float InHalfHeight,
        float InDamage, float InPoiseDamage,
        UParticleSystem* InHitFX = nullptr,
        const FVector& InOverrideDirection = FVector::ZeroVector);

    //--- Subjugation ---------------------------------------------------------

    TArray<FVector> PendingSubjugationLocations;

    UPROPERTY()
    TObjectPtr<USoundBase> PendingSwirlsSFX;

    FTimerHandle SubjugationDamageTimerHandle;

    static TArray<FVector> GenerateSwirlLocations(const FVector& InOrigin,
        float InSpreadRadius, float InMinSeparation, int32 InCount);

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
    void TryDestroyActor();

    UPROPERTY()
    TArray<TObjectPtr<UMaterialInstanceDynamic>> DissolveMIDs;

    FTimerHandle DissolveTimerHandle;
    float        DissolveStartTime       = 0.f;
    bool         bDissolveCompleted      = false;
    bool         bDeathMontageBlendedOut = false;

    void OnGroggySequenceCompleted();
    void OnAttackMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);
    void OnSingleShotActionBlendingOut(UAnimMontage* InMontage, bool bInterrupted);
    void CheckPhaseTransition(float InNewHealthRatio);
    void OnPoiseDepleted();
};
