// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RVInputConfig.generated.h"

class UInputAction;

/**
 * 
 */
UCLASS(BlueprintType)
class REVENANT_API URVInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	// Move (2D Axis - WASD)
	UPROPERTY(EditDefaultsOnly, Category = "RV|Input")
	TObjectPtr<UInputAction> MoveAction;
    
    
	//  Look (2D Axis - Mouse(X Y)
	UPROPERTY(EditDefaultsOnly, Category = "RV|Input")
	TObjectPtr<UInputAction> LookAction;
    
	// Jump (Button)
	UPROPERTY(EditDefaultsOnly, Category = "RV|Input")
	TObjectPtr<UInputAction> JumpAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "RV|Input")
	TObjectPtr<UInputAction> AttackAction;
	
};

