// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RVCharacterDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class REVENANT_API URVCharacterDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BluePrintReadOnly, Category = "RV|Attribute")
	float MaxHealth = 100.0f;
	
	UPROPERTY(EditDefaultsOnly, BluePrintReadOnly, Category = "RV|Attribute")
	float MaxStamina = 100.0f;
	
	UPROPERTY(EditDefaultsOnly, BluePrintReadOnly, Category = "RV|Attribute")
	float StaminaRegenRate = 20.f;
	
	UPROPERTY(EditDefaultsOnly, BluePrintReadOnly, Category = "RV|Attribute")
	float AttackDamage = 20.f;
};
