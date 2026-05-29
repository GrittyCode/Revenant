#pragma once

#include "CoreMinimal.h"
#include "Character/Base/RVCharacterBase.h"
#include "Component/RVEquipmentComponent.h"
#include "Data/RVPlayerDataAsset.h"
#include "Component/RVStaminaComponent.h"
#include "RVCharacterPlayer.generated.h"

class URVInputConfig;
class UInputMappingContext;
class USpringArmComponent;
class UCameraComponent;
class URVWeaponDataAsset;
class URVLockOnComponent;
class URVWeaponAttackComponent;
class URVGuardComponent;
class URVHitReactionAnimDataAsset;
class URVPlayerDataAsset;

struct FInputActionValue;

UCLASS()
class REVENANT_API ARVCharacterPlayer : public ARVCharacterBase
{
    GENERATED_BODY()

public:
    ARVCharacterPlayer();

    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual bool ApplyDamage(const FRVHitInfo& InHitInfo) override;

    //--- AnimInstance accessors ----------------------------------------------

    bool  IsComboActive()  const;
    float GetSprintSpeed() const;
    bool  IsSprinting()    const;
    bool  IsLockedOn()     const;

    //--- Stamina queries (external: PlayerController, HUD) -------------------
	
	float GetStaminaRatio() const;

    FRVOnStaminaChanged& GetOnStaminaChanged();

    URVStaminaComponent* GetStaminaComponent() const { return StaminaComponent; }

    //--- AnimNotify forwarding -----------------------------------------------

    void OpenComboWindow();
    void CloseComboWindow();
    void TryChainNextCombo();
    void SetHeavyAttackReady(bool bReady);

    //--- Equipment -----------------------------------------------------------

    FRVOnWeaponChanged& GetOnWeaponChanged();
    URVEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent; }

protected:
    virtual void BeginPlay()  override;
    virtual void OnDeath()    override;
    virtual void InitStats()  override;
    virtual void Landed(const FHitResult& Hit) override;

    virtual URVHitReactionAnimDataAsset* GetHitReactionAnimData() const override;
    virtual UMeshComponent* GetWeaponTraceMesh()   const override;
    virtual void ActivateWeaponTrail()   override;
    virtual void DeactivateWeaponTrail() override;

    UFUNCTION()
    void OnWeaponChangedHandler(URVWeaponDataAsset* NewWeaponData);

    //--- Player-only components ----------------------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVStaminaComponent> StaminaComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVWeaponAttackComponent> WeaponAttackComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVGuardComponent> GuardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVEquipmentComponent> EquipmentComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
	TObjectPtr<URVLockOnComponent> LockOnComponent;

private:
    //--- Input ---------------------------------------------------------------

    void InputMove  (const FInputActionValue& Value);
    void InputLook  (const FInputActionValue& Value);
    void InputJump  (const FInputActionValue& Value);
    void InputAttack(const FInputActionValue& Value);
    void InputHeavyAttackStarted  (const FInputActionValue& Value);
    void InputHeavyAttackCompleted(const FInputActionValue& Value);
    void InputDodge(const FInputActionValue& Value);
    void InputSprintStarted  (const FInputActionValue& Value);
    void InputSprintCompleted(const FInputActionValue& Value);
    void InputGuardStarted  (const FInputActionValue& Value);
    void InputGuardCompleted(const FInputActionValue& Value);
    void InputLockOn(const FInputActionValue& Value);
    void InputWeaponSwap(const FInputActionValue& Value);

    //--- Attack direction ----------------------------------------------------

    void  SnapToAttackDirection();
    float AttackStartYaw = 0.f;

    //--- Sprint --------------------------------------------------------------

    void StartSprint();
    void EndSprint();
    void OnCombatStateChangedForSprint(ERVCombatState InNewState);

    UPROPERTY(EditDefaultsOnly, Category = "RV|Sprint")
    float SprintSpeed = 1000.f;

    float OriginalWalkSpeed = 0.f;
    bool  bIsSprinting      = false;

    //--- Dodge ---------------------------------------------------------------

    bool CanStartDodge() const;
    void StartDodge(UAnimMontage* InMontage);
    void EndDodge();

    UFUNCTION()
    void OnDodgeMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);

    UPROPERTY()
    TObjectPtr<UAnimMontage> ActiveDodgeMontage;

    float CachedDodgeStaminaCost = 30.f;

    //--- Config --------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "RV|Data")
    TObjectPtr<URVPlayerDataAsset> PlayerData;

    UPROPERTY(EditDefaultsOnly, Category = "RV|Input")
    TObjectPtr<URVInputConfig> InputConfig;

    UPROPERTY(EditDefaultsOnly, Category = "RV|Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    //--- Camera --------------------------------------------------------------

    UPROPERTY(VisibleAnywhere, Category = "RV|Components")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, Category = "RV|Components")
    TObjectPtr<UCameraComponent> FollowCamera;

    //--- Attack rotation -----------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "RV|Combat")
    float AttackRotationInterpSpeed = 10.f;
};