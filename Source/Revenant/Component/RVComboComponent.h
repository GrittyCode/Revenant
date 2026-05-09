#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVComboComponent.generated.h"

class URVEquipmentComponent;
class URVAttributeComponent;
class URVCombatStateComponent;

// Fired on combo start/end — URVCombatStateComponent subscribes to manage Attacking bit
DECLARE_MULTICAST_DELEGATE(FRVOnComboStarted);
DECLARE_MULTICAST_DELEGATE(FRVOnComboEnded);

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVComboComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVComboComponent();

    /**
     * Entry point for all combo input — called directly by ARVCharacterPlayer::InputAttack.
     * Performs gate checks (grounded, CheckAvailableState) via CombatStateComponent,
     * then starts a new combo or buffers continuation input.
     */
    void HandleComboInput();

    /**
     * Called by UAnimNotifyState_ComboWindow::NotifyBegin.
     * Opens the input acceptance window — only inputs received during this window
     * are treated as valid combo continuations.
     */
    void OpenComboWindow();

    /**
     * Called by UAnimNotifyState_ComboWindow::NotifyEnd.
     * Closes the input window and resolves the buffered input:
     * advances to the next section if input was buffered, ends combo otherwise.
     */
    void CloseComboWindow();

    UFUNCTION(BlueprintCallable, Category = "RV|Combo")
    bool IsComboActive() const { return bIsComboActive; }

    UFUNCTION(BlueprintCallable, Category = "RV|Combo")
    int32 GetComboCount() const { return ComboCount; }

    // URVCombatStateComponent subscribes to these to keep Attacking bit in sync
    FRVOnComboStarted OnComboStarted;
    FRVOnComboEnded   OnComboEnded;

protected:
    virtual void BeginPlay() override;

private:
    // --- Cached References ---------------------------------------------------

    UPROPERTY()
    TObjectPtr<URVEquipmentComponent> EquipmentComponent;

    UPROPERTY()
    TObjectPtr<URVAttributeComponent> AttributeComponent;

    // Higher-level orchestrator: ComboComponent asks CombatStateComponent for gate state.
    UPROPERTY()
    TObjectPtr<URVCombatStateComponent> CombatStateComponent;

    // Cached to avoid repeated Cast<ACharacter>(GetOwner()) in hot paths.
    UPROPERTY()
    TObjectPtr<ACharacter> OwnerCharacter;

    // --- State ---------------------------------------------------------------

    bool  bIsComboActive  = false;
    bool  bHasComboInput  = false;
    bool  bComboWindowOpen = false;
    int32 ComboCount       = 0;

    // --- Internal ------------------------------------------------------------

    void StartCombo();
    void EndCombo();

    /** Plays the montage section for the current ComboCount. */
    void PlayComboSection();

    /**
     * Called by CloseComboWindow. Advances to the next section if input was buffered,
     * ends combo otherwise.
     */
    void TryAdvanceCombo();

    /**
     * Called via URVCombatStateComponent::OnForceEnd delegate.
     * Stops the attack montage and resets all combo state atomically.
     */
    void ForceEndCombo();

    void OnComboMontageEnded(UAnimMontage*, bool bInterrupted);
};