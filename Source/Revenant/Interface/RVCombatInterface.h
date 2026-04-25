// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RVCombatInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class URVCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class REVENANT_API IRVCombatInterface
{
	GENERATED_BODY()

public:
	// Called by AnimNotifty_AttackHitCheck at the exact frame weapon should deal damage.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RV|Combat")
	void ActivateHitCheck();
};
