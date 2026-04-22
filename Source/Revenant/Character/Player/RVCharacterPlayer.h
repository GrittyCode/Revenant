#pragma once

#include "CoreMinimal.h"
#include "Character/Base/RVCharacterBase.h"
#include "RVCharacterPlayer.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class URVInputConfig;
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
	// Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UCameraComponent> Camera;

	// Input Config DataAsset — IMC와 InputAction 레퍼런스 보관
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Input", meta = (AllowPrivateAccess = true))
	TObjectPtr<URVInputConfig> InputConfig;

	// Input handlers
	void Move(const FInputActionValue& InValue);
	void Look(const FInputActionValue& InValue);
	void HandleAttackInput(const FInputActionValue& InValue);
};