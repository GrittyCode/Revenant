#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "RVWeaponAttackComponent.generated.h"

class ARVCharacterBase;
class ARVCharacterPlayer;
class URVStaminaComponent;
class URVEquipmentComponent;
class UAnimMontage;

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVWeaponAttackComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVWeaponAttackComponent();

    void HandleLightAttackInput(bool bIsPlayerSprinting);
    void TryChainNextCombo();
    void OpenComboWindow();
    void CloseComboWindow();
    void OnPlayerLanded();

    bool IsLightAttackActive() const { return bIsLightAttackActive; }
    bool IsJumpAttackLanding() const { return bIsJumpAttackLanding; }

    void StartHeavyAttack();
    void ReleaseHeavyAttack();
    void SetHeavyAttackReady(bool bReady);
    void ForceEndAttack();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TObjectPtr<ARVCharacterBase> OwnerBase;

    // Cached from OwnerBase — set once in BeginPlay after player cast is verified.
    UPROPERTY()
    TObjectPtr<URVStaminaComponent> StaminaComponent;

    UPROPERTY()
    TObjectPtr<URVEquipmentComponent> EquipmentComponent;

    //--- Light Attack State --------------------------------------------------

    bool bIsLightAttackActive = false;
    bool bComboWindowOpen     = false;
    bool bHasComboInput       = false;
    bool bHasUsedJumpAttack   = false;
    bool bIsJumpAttackActive  = false;
    bool bIsJumpAttackLanding = false;

    TEnumAsByte<ERootMotionMode::Type> CachedRootMotionMode =
        ERootMotionMode::RootMotionFromMontagesOnly;

    void StartCombo();
    void StartRunAttack();
    void StartJumpAttack();
    void EndCombo();
    void PlayLightAttackMontage(UAnimMontage* InMontage);

    bool ConsumeAttackStamina(UAnimMontage* InMontage);

    UAnimInstance* GetAnimInstance() const;

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

    void OnChargeMontageBlendingOut (UAnimMontage*, bool bInterrupted);
    void OnReleaseMontageBlendingOut(UAnimMontage*, bool bInterrupted);
};
