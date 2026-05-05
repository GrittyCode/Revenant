#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVComboComponent.generated.h"

class URVEquipmentComponent;
class URVAttributeComponent;

// Fired on combo start/end — URVCombatComponent subscribes to manage bIsAttacking flag
DECLARE_MULTICAST_DELEGATE(FRVOnComboStarted);
DECLARE_MULTICAST_DELEGATE(FRVOnComboEnded);

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVComboComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVComboComponent();

    /**
     * Called by URVCombatComponent::TryStartCombo() after all gate checks pass.
     * Starts the first combo hit or buffers the next hit if already attacking.
     * Caller is responsible for grounded check and CanPerformAction.
     */
    void HandleComboInput();

    /**
     * Called by AnimNotify_ComboWindow.
     * Advances to the next combo section if input was buffered.
     */
    void TryAdvanceCombo();

    UFUNCTION(BlueprintCallable, Category = "RV|Combo")
    bool IsComboActive() const { return bIsComboActive; }

    UFUNCTION(BlueprintCallable, Category = "RV|Combo")
    int32 GetComboCount() const { return ComboCount; }

    // URVCombatComponent subscribes to these to keep bIsAttacking in sync
    FRVOnComboStarted OnComboStarted;
    FRVOnComboEnded   OnComboEnded;

protected:
    virtual void BeginPlay() override;

private:
    // --- Cached Component References --------------------------------------------

    UPROPERTY()
    TObjectPtr<URVEquipmentComponent> EquipmentComponent;

    UPROPERTY()
    TObjectPtr<URVAttributeComponent> AttributeComponent;

    // --- State ------------------------------------------------------------------

    bool  bIsComboActive = false;
    bool  bHasComboInput = false;
    int32 ComboCount     = 0;

    // --- Internal ---------------------------------------------------------------

    void StartCombo();
    void EndCombo();

    /** Plays the montage section for the current ComboCount. */
    void PlayComboSection();

    /** Bound to montage blend-out — cleans up when full combo ends or is interrupted. */
    void OnComboMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);
};