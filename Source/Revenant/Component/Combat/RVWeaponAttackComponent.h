#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVWeaponAttackComponent.generated.h"

class ARVCharacterBase;
class URVStaminaComponent;
class URVEquipmentComponent;
class UAnimMontage;

UCLASS(ClassGroup=(Revenant))
class REVENANT_API URVWeaponAttackComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVWeaponAttackComponent();
    virtual void BeginPlay() override;

    // Called from ARVCharacterPlayer::BeginPlay after all components exist.
    void Init(URVStaminaComponent* InStamina, URVEquipmentComponent* InEquipment);

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

private:
    UPROPERTY()
    TObjectPtr<ARVCharacterBase> OwnerBase;

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

    UAnimInstance* GetAnimInstance() const;

    void StartCombo();
    void StartRunAttack();
    void StartJumpAttack();
    void EndCombo();
    void PlayLightAttackMontage(UAnimMontage* InMontage);
    bool ConsumeAttackStamina(UAnimMontage* InMontage);

    void OnLightAttackMontageBlendingOut(UAnimMontage*, bool);

    //--- Heavy Attack State --------------------------------------------------

    enum class EHeavyPhase : uint8
    {
        None,
        Charging,
        Releasing,
    };

    EHeavyPhase HeavyPhase = EHeavyPhase::None;

    FTimerHandle ChargeAutoReleaseHandle;

    bool bCanHeavyRelease = false;
    bool bPendingRelease  = false;
    bool bIsAutoRelease   = false;

    void ExecuteHeavyAttack();
    void EndHeavyAttack();
    void OnChargeAutoRelease();

    void OnChargeMontageBlendingOut (UAnimMontage*, bool);
    void OnReleaseMontageBlendingOut(UAnimMontage*, bool);
};
