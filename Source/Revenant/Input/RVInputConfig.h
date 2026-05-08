// Source/Revenant/Input/RVInputConfig.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RVInputConfig.generated.h"

class UInputAction;

UCLASS()
class REVENANT_API URVInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "RV|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Input")
	TObjectPtr<UInputAction> AttackAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "RV|Input")
	TObjectPtr<UInputAction> HeavyAttackAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "RV|Input")
	TObjectPtr<UInputAction> DodgeAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "RV|Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Input")
	TObjectPtr<UInputAction> GuardAction;
	
	/** Left Shift — used as Chord modifier for heavy attack. */
	UPROPERTY(EditDefaultsOnly, Category = "RV|Input")
	TObjectPtr<UInputAction> HeavyModifierAction;
};