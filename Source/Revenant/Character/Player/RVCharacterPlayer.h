// Source/Revenant/Character/Player/RVCharacterPlayer.h
#pragma once

#include "CoreMinimal.h"
#include "Character/Base/RVCharacterBase.h"
#include "RVCharacterPlayer.generated.h"

class URVEquipmentComponent;
class URVInputConfig;
class UInputMappingContext;
class USpringArmComponent;
class UCameraComponent;
class URVWeaponDataAsset;
class URVLockOnComponent;
class URVComboComponent;
class URVHeavyAttackComponent;
class URVDodgeComponent;
class URVGuardComponent;
class URVSprintComponent;
class URVHitReactionAnimDataAsset;

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

protected:
	virtual void BeginPlay() override;
	virtual void OnDeath() override;

	virtual URVHitReactionAnimDataAsset* GetHitReactionAnimData() const override;
	virtual UMeshComponent* GetWeaponTraceMesh() const override;

	UFUNCTION()
	void OnWeaponChangedHandler(URVWeaponDataAsset* NewWeaponData);

	// --- Player-only Action Components ---------------------------------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
	TObjectPtr<URVComboComponent> ComboComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
	TObjectPtr<URVHeavyAttackComponent> HeavyAttackComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
	TObjectPtr<URVDodgeComponent> DodgeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
	TObjectPtr<URVGuardComponent> GuardComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
	TObjectPtr<URVSprintComponent> SprintComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
	TObjectPtr<URVEquipmentComponent> EquipmentComponent;
	

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

	void InputLockOn(const FInputActionValue& Value);

	void InputWeaponSwap(const FInputActionValue& Value);

	// -- Attack Direction -----------------------------------------------------

	// Snaps rotation to target (lock-on) or freezes current yaw (default).
	// Called at attack start to prevent mid-combo camera steering.
	void SnapToAttackDirection();
	
	float AttackStartYaw = 0.f;

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

	// -- Lock-on --------------------------------------------------------------

	UPROPERTY(VisibleAnywhere, Category = "RV|Components")
	TObjectPtr<URVLockOnComponent> LockOnComponent;

	// -- Attack Rotation ------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "RV|Combat")
	float AttackRotationInterpSpeed = 10.f;
};
