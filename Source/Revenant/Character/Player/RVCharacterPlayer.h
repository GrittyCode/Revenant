#pragma once

#include "CoreMinimal.h"
#include "Character/Base/RVCharacterBase.h"
#include "Component/RVEquipmentComponent.h"
#include "RVCharacterPlayer.generated.h"

class URVInputConfig;
class UInputMappingContext;
class USpringArmComponent;
class UCameraComponent;
class URVWeaponDataAsset;
class URVLockOnComponent;
class URVWeaponAttackComponent;
class URVDodgeComponent;
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

    //--- Component facades (AnimInstance uses these) -------------------------

    FRVOnWeaponChanged& GetOnWeaponChanged();
    URVWeaponDataAsset* GetCurrentWeaponData() const;
    bool  IsComboActive()  const;
    float GetSprintSpeed() const;
    bool  IsSprinting()    const;
    bool  IsLockedOn()     const;

protected:
    virtual void BeginPlay() override;
    virtual void OnDeath() override;
    virtual void InitStats() override;
    virtual void Landed(const FHitResult& Hit) override;

    virtual URVHitReactionAnimDataAsset* GetHitReactionAnimData() const override;
    virtual UMeshComponent* GetWeaponTraceMesh() const override;

    UFUNCTION()
    void OnWeaponChangedHandler(URVWeaponDataAsset* NewWeaponData);

    //--- Player-only Action Components ---------------------------------------

    // Handles all weapon attack actions: combo, run attack, jump attack, heavy attack.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVWeaponAttackComponent> WeaponAttackComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVDodgeComponent> DodgeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVGuardComponent> GuardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVEquipmentComponent> EquipmentComponent;

private:
    //--- Input Handlers ------------------------------------------------------

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

    //--- Attack Direction ----------------------------------------------------

    void SnapToAttackDirection();

    float AttackStartYaw = 0.f;

    //--- Input Config --------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "RV|Input")
    TObjectPtr<URVInputConfig> InputConfig;

    UPROPERTY(EditDefaultsOnly, Category = "RV|Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    //--- Camera --------------------------------------------------------------

    UPROPERTY(VisibleAnywhere, Category = "RV|Components")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, Category = "RV|Components")
    TObjectPtr<UCameraComponent> FollowCamera;

    //--- Lock-on -------------------------------------------------------------

    UPROPERTY(VisibleAnywhere, Category = "RV|Components")
    TObjectPtr<URVLockOnComponent> LockOnComponent;

    //--- Attack Rotation -----------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "RV|Combat")
    float AttackRotationInterpSpeed = 10.f;

    //--- Sprint --------------------------------------------------------------

    void StartSprint();
    void EndSprint();
    void OnCombatStateChangedForSprint(ERVCombatState InNewState);

    UPROPERTY(EditDefaultsOnly, Category = "RV|Sprint")
    float SprintSpeed = 1000.f;

    float OriginalWalkSpeed = 0.f;
    bool  bIsSprinting      = false;
};