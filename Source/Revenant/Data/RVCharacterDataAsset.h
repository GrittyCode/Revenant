#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/RVWeaponDataAsset.h"
#include "RVCharacterDataAsset.generated.h"

class UAnimMontage;

UCLASS()
class REVENANT_API URVCharacterDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // ─── HP / Stamina ────────────────────────────────────────────────────────

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Attribute")
    float MaxHealth = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Attribute")
    float MaxStamina = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Attribute")
    float StaminaRegenRate = 20.f;

    /** Seconds before stamina regen begins after an action. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Attribute")
    float StaminaRegenDelay = 1.5f;

    // ─── Poise ───────────────────────────────────────────────────────────────

    /**
     * Maximum poise value. Loaded into URVAttributeComponent at BeginPlay.
     * Poise is depleted by incoming hits (PoiseDamage per hit) and reset after
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Poise")
    float MaxPoise = 100.f;

    /**
     * Number of stagger triggers before Groggy is entered instead.
     * StaggerCount is tracked in URVHitReactionComponent and resets on Groggy entry.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Poise",
              meta = (ClampMin = "1"))
    int32 GroggyThreshold = 2;

    /** Seconds Groggy state lasts before auto-recovering. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Poise")
    float GroggyDuration = 3.f;

    // ─── Hit Reaction Montages ───────────────────────────────────────────────
    //
    // Directional stagger montages — played based on which direction the attack
    // came from (relative to target's forward vector).

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|HitReaction")
    TObjectPtr<UAnimMontage> StaggerMontage_F;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|HitReaction")
    TObjectPtr<UAnimMontage> StaggerMontage_B;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|HitReaction")
    TObjectPtr<UAnimMontage> StaggerMontage_L;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|HitReaction")
    TObjectPtr<UAnimMontage> StaggerMontage_R;

    /**
     * Groggy montage. Played on Groggy entry and stopped when GroggyDuration expires.
     * Should be a looping or sufficiently long montage to cover GroggyDuration.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|HitReaction")
    TObjectPtr<UAnimMontage> GroggyMontage;

    /** Played when the character is knocked down. Transitions to GetUpMontage on blend-out. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|HitReaction")
    TObjectPtr<UAnimMontage> KnockdownMontage;

    /** Played after KnockdownMontage blends out. Clearing Knockdown state on completion. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|HitReaction")
    TObjectPtr<UAnimMontage> GetUpMontage;
};