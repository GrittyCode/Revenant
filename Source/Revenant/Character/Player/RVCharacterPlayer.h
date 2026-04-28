// Source/Revenant/Character/Player/RVCharacterPlayer.h
#pragma once

#include "CoreMinimal.h"
#include "Character/Base/RVCharacterBase.h"
#include "RVCharacterPlayer.generated.h"

class URVInputConfig;
class UInputMappingContext;
class USpringArmComponent;
class UCameraComponent;

struct FInputActionValue;

UCLASS()
class REVENANT_API ARVCharacterPlayer : public ARVCharacterBase 
{
	GENERATED_BODY()

public:
	ARVCharacterPlayer();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	// -- Input Handlers ---------------------------------------------------------

	void InputMove  (const FInputActionValue& Value);
	void InputLook  (const FInputActionValue& Value);
	void InputJump  (const FInputActionValue& Value);
	void InputAttack(const FInputActionValue& Value);

	void InputDodge(const FInputActionValue& Value);

	void InputSprintStarted  (const FInputActionValue& Value);
	void InputSprintCompleted(const FInputActionValue& Value);

	void InputGuardStarted  (const FInputActionValue& Value);
	void InputGuardCompleted(const FInputActionValue& Value);

	// -- Input Config ---------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "RV|Input")
	TObjectPtr<URVInputConfig> InputConfig;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	
	// -- Camera Config ---------------------------------------------------------
	
	UPROPERTY(VisibleAnywhere, Category = "RV|Components")
	TObjectPtr<USpringArmComponent> CameraBoom;
	
	UPROPERTY(VisibleAnywhere, Category = "RV|Components")
	TObjectPtr<UCameraComponent> FollowCamera;
	
};