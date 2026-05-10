#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/RVDamageable.h"
#include "RVHitReactionComponent.generated.h"

class ACharacter;
class URVAttributeComponent;
class URVCombatStateComponent;
class URVCharacterDataAsset;
class UAnimMontage;

/**
 * Handles all incoming hit responses for a character:
 *   - Physical Reaction  : passes HitDirection to URVAnimInstance every hit (ABP additive layer)
 *   - Stagger            : poise depleted, not airborne, StaggerCount < GroggyThreshold
 *   - Groggy             : poise depleted, StaggerCount reaches GroggyThreshold
 *   - Knockdown          : poise depleted + (airborne OR bForceKnockdown in FRVHitInfo)
 *
 * Entry point: HandleHit(FRVHitInfo) — called by ARVCharacterBase::ApplyDamage
 * after HP damage has been applied.
 *
 * Dependency direction:
 *   URVHitReactionComponent → URVCombatStateComponent  (state write / gate read)
 *   URVHitReactionComponent → URVAttributeComponent    (Poise depletion / regen)
 *   URVHitReactionComponent → URVAnimInstance          (physical reaction trigger)
 */
UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVHitReactionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVHitReactionComponent();

    /**
     * Main entry point. Called by ARVCharacterBase::ApplyDamage after HP is applied.
     * Handles physical reaction (always), then poise-based reaction if poise depletes.
     */
    void HandleHit(const FRVHitInfo& InHitInfo);

    /** Reference injection — called by ARVCharacterBase::BeginPlay (Composition Root). */
    void InitReferences(ACharacter* InOwnerCharacter,
                        URVCombatStateComponent* InCombatStateComponent,
                        URVAttributeComponent* InAttributeComponent,
                        URVCharacterDataAsset* InCharacterData);

protected:
    virtual void BeginPlay() override;

private:
    // --- Cached References ---------------------------------------------------

    UPROPERTY()
    TObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<URVCombatStateComponent> CombatStateComponent;

    UPROPERTY()
    TObjectPtr<URVAttributeComponent> AttributeComponent;

    // Raw pointer — DataAsset lifetime is bound to the owning character Blueprint.
    // URVCharacterDataAsset is an EditDefaultsOnly asset; valid for the character's lifetime.
    UPROPERTY()
    TObjectPtr<URVCharacterDataAsset> CharacterData;

    // --- Stagger Accumulation ------------------------------------------------

    /**
     * Counts consecutive poise depletions that resulted in Stagger.
     * When StaggerCount >= CharacterData->GroggyThreshold, the next poise
     * depletion triggers Groggy instead of Stagger, and StaggerCount resets.
     */
    int32 StaggerCount = 0;

    // --- Timers --------------------------------------------------------------

    /** Controls Groggy duration. Calls EndGroggy when it expires. */
    FTimerHandle GroggyHandle;

    // --- Reaction Triggers ---------------------------------------------------

    /**
     * Triggers the ABP physical reaction additive layer.
     * Called on every hit regardless of poise state.
     */
    void TriggerPhysicalReaction(const FVector& InHitDirection);

    /**
     * Selects and plays a directional stagger montage based on where the hit
     * came from (relative to this character's forward vector).
     * Sets HitReaction state; clears on montage blend-out.
     */
    void TriggerStagger(const FVector& InHitDirection);

    /**
     * Plays GroggyMontage and starts the GroggyDuration timer.
     * Clears Groggy state and resumes stamina regen when timer expires.
     */
    void TriggerGroggy();

    /**
     * Plays KnockdownMontage. On blend-out, plays GetUpMontage.
     * Clears Knockdown state after get-up completes.
     */
    void TriggerKnockdown();

    // --- Montage End Callbacks -----------------------------------------------

    UFUNCTION()
    void OnStaggerMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void EndGroggy();

    UFUNCTION()
    void OnKnockdownMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void OnGetUpMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

    // --- Helpers -------------------------------------------------------------

    /**
     * Classifies InHitDirection into a directional stagger montage.
     * InHitDirection: world-space from instigator toward target.
     * Returns the matching StaggerMontage_F/B/L/R from CharacterData.
     * Falls back to StaggerMontage_F if the directional slot is null.
     */
    UAnimMontage* SelectStaggerMontage(const FVector& InHitDirection) const;
};