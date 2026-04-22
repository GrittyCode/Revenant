// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RVWeaponDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class REVENANT_API URVWeaponDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Combo hit count - drives ComboSectionNames length
	// Change this value + add a section to the montage to extend the combo
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Combo")
	int32 MaxComboCount = 3;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Combo")
	TArray<FName> ComboSectionNames;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Combo")
	float AttackStaminaCost = 20.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Combo")
	TObjectPtr<UAnimMontage> AttackMontage;
	
};
