#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "RVWeaponAttackComponent.generated.h"

class ACharacter;
class URVAttributeComponent;
class URVEquipmentComponent;
class URVWeaponDataAsset;
class UAnimMontage;

DECLARE_MULTICAST_DELEGATE(FRVOnLightAttackStarted);
DECLARE_MULTICAST_DELEGATE(FRVOnLightAttackEnded);

/**
 * Handles all weapon attack actions for the player:
 * light attack (combo / run / jump) and heavy attack (charge / release).
 * All attack data is read from the active URVWeaponDataAsset,
 * which is the natural owner of per-weapon attack definitions.
 */
UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVWeaponAttackComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVWeaponAttackComponent();

    void InitReferences(ACharacter* InOwnerCharacter,
                        URVCombatStateComponent* InCombatStateComponent,
                        URVAttributeComponent* InAttributeComponent,
                        URVEquipmentComponent* InEquipmentComponent);

    //--- Light Attack --------------------------------------------------------

    // Entry point for light attack input (combo / run attack / jump attack).
    // bIsPlayerSprinting is passed from ARVCharacterPlayer — the character owns sprint state.
    void HandleLightAttackInput(bool bIsPlayerSprinting);

    // Called by AnimNotify_ComboChain — chains to the next combo hit or ends combo.
    void TryChainNextCombo();

    // Called by AnimNotifyState_ComboWindow Begin/End.
    void OpenComboWindow();
    void CloseComboWindow();

    // Called by ARVCharacterPlayer::Landed.
    // Transitions the jump attack Loop → Landing section if active,
    // and resets the one-per-jump gate for the next airborne session.
    void OnPlayerLanded();

    bool IsComboActive() const { return bIsComboActive; }
    bool IsJumpAttackLanding() const { return bIsJumpAttackLanding; }

    //--- Heavy Attack --------------------------------------------------------

    // Begins heavy charge. Plays HeavyChargeMontage (loops until released).
    void StartHeavyAttack();

    /**
     * Requests heavy release.
     * Buffered in bPendingRelease if Loop section not yet reached.
     */
    void ReleaseHeavyAttack();

    /**
     * Called by AnimNotify_HeavyAttackReady on Loop section entry.
     * Activates release gating and flushes buffered release.
     */
    void SetHeavyAttackReady(bool bReady);

    //--- Shared --------------------------------------------------------------

    // Called via CombatStateComponent::OnForceEnd.
    void ForceEndAttack();

    //--- Delegates -----------------------------------------------------------

    // Fired when a light attack starts — CombatStateComponent subscribes to set Attacking bit.
    FRVOnLightAttackStarted OnLightAttackStarted;
    FRVOnLightAttackEnded   OnLightAttackEnded;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<URVCombatStateComponent> CombatStateComponent;

    UPROPERTY()
    TObjectPtr<URVAttributeComponent> AttributeComponent;

    UPROPERTY()
    TObjectPtr<URVEquipmentComponent> EquipmentComponent;

    //--- Light Attack State --------------------------------------------------

    bool bIsComboActive      = false;
    bool bComboWindowOpen    = false;
    bool bHasComboInput      = false;
    bool bHasUsedJumpAttack  = false;
    bool bIsJumpAttackActive  = false; // true while jump attack Begin/Loop is playing
    bool bIsJumpAttackLanding = false; // true during Landing section — blocks movement input

    // Root motion mode before jump attack overrides it — restored on EndCombo.
    TEnumAsByte<ERootMotionMode::Type> CachedRootMotionMode =
        ERootMotionMode::RootMotionFromMontagesOnly;

    void StartCombo();
    void StartRunAttack();
    void StartJumpAttack();
    void EndCombo();
    void PlayLightAttackMontage(UAnimMontage* InMontage);

    // Returns false if stamina was insufficient — caller should abort the attack.
    bool ConsumeAttackStamina(UAnimMontage* InMontage, const URVWeaponDataAsset* InWeaponData);

    UFUNCTION()
    void OnLightAttackMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

    //--- Heavy Attack State --------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "RV|HeavyAttack")
    float MaxChargeTime = 1.5f;

    FTimerHandle ChargeAutoReleaseHandle;

    bool bCanHeavyRelease = false;
    bool bPendingRelease  = false;
    bool bIsAutoRelease   = false;

    void ExecuteHeavyAttack();
    void EndHeavyAttack();

    UFUNCTION()
    void OnChargeAutoRelease();

    void OnChargeMontageBlendingOut(UAnimMontage*, bool bInterrupted);
    void OnReleaseMontageBlendingOut(UAnimMontage*, bool bInterrupted);
};