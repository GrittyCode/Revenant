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
	UPROPERTY(EditAnywhere, Category = "RV|Input")
	TObjectPtr<UInputAction> MoveAction;
    
    
	//  Look (2D Axis - Mouse(X Y)
	UPROPERTY(EditAnywhere, Category = "RV|Input")
	TObjectPtr<UInputAction> LookAction;
    
	// Jump (Button)
	UPROPERTY(EditAnywhere, Category = "RV|Input")
	TObjectPtr<UInputAction> JumpAction;
};

