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
	
	
	// --- Attack  --------------------------------------------
	
	UPROPERTY(EditDefaultsOnly, Category = "RV|Attack")
	float AttackDamage = 30.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "RV|Attack")
	float AttackRange;
	
	UPROPERTY(EditDefaultsOnly, Category = "RV|Attack")
	float AttackRadius;
	
	// --- Combo --------------------------------------------------
	
	UPROPERTY(EditDefaultsOnly, Category = "RV|Combo")
	int32 MaxComboCount = 4;
	
	/* Section names in play order : Section_1, Section_2 ....   */
	UPROPERTY(EditDefaultsOnly, Category = "RV|Combo")
	TArray<FName> ComboSectionNames;
	
	UPROPERTY(EditDefaultsOnly, Category = "RV|Combo")
	TObjectPtr<UAnimMontage> AttackMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = "RV|Combo")
	float AttackStaminaCost = 20.f;
	
	
	// --- Guard ------------------------------------------------------
	
	/*
	 *	Guard Montage - contain three sections in order :
	 *	Begin -> Loop (looping) -> End 
	 */
	
	UPROPERTY(EditDefaultsOnly, Category = "RV|Guard")
	TObjectPtr<UAnimMontage> GuardMontage;
	
	/* Play when Stamina reaches 0 while guarding */
	UPROPERTY(EditDefaultsOnly, Category = "RV|Guard")
	TObjectPtr<UAnimMontage> GuardBreakMontage;
	
	
	// --- Dodge --------------------------------------------------------
	
	/*
	 * Dodge Montages indexed by input direction
	 * [0] = Forward, [1] = Backward, [2] = Left, [3] = Right
	 * Assign all four in the DataAsset. URVCombatComponent selects by dot product
	 */ 
	
	UPROPERTY(EditDefaultsOnly, Category = "RV|Dodge")
	TArray<TObjectPtr<UAnimMontage>> DodgeMontages;
	
	UPROPERTY(EditDefaultsOnly, Category = "RV|Dodge")
	float DodgeStaminaCost = 30.f;
	
	
	
};
