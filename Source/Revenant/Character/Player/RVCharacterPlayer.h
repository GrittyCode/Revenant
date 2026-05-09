#pragma once

#include "CoreMinimal.h"
#include "Character/Base/RVCharacterBase.h"
#include "RVCharacterPlayer.generated.h"

class URVInputConfig;
class UInputMappingContext;
class USpringArmComponent;
class UCameraComponent;
class URVWeaponDataAsset;
class URVSprintComponent;

struct FInputActionValue;

UCLASS()
class REVENANT_API ARVCharacterPlayer : public ARVCharacterBase
{
    GENERATED_BODY()

public:
    ARVCharacterPlayer();

    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
    virtual void BeginPlay() override;

private:
    // -- Input Handlers -------------------------------------------------------

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

    // Phase 2 only — replaced by ARVWeaponPickup overlap in Phase 4.
    void InputWeaponSwap(const FInputActionValue& Value);

    // -- Input Config ---------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "RV|Input")
    TObjectPtr<URVInputConfig> InputConfig;

    UPROPERTY(EditDefaultsOnly, Category = "RV|Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    // -- Camera ---------------------------------------------------------------

    UPROPERTY(VisibleAnywhere, Category = "RV|Components")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, Category = "RV|Components")
    TObjectPtr<UCameraComponent> FollowCamera;

    // -- Attack Rotation ------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "RV|Combat")
    float AttackRotationInterpSpeed = 10.f;

    // -- Weapon Swap (Phase 2 temp) -------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "RV|Weapon")
    TObjectPtr<URVWeaponDataAsset> WeaponDataA;

    UPROPERTY(EditDefaultsOnly, Category = "RV|Weapon")
    TObjectPtr<URVWeaponDataAsset> WeaponDataB;

    bool bIsWeaponA = true;
};