// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RVWeaponStyleDataAsset.generated.h"

/**
 * Shared animation and locomotion assets for all weapons of the same type.
 */
UCLASS()
class REVENANT_API URVWeaponStyleDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	// -- Montages --
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation")
	TObjectPtr<UAnimMontage> DodgeMontage;

	/** Played when guard break stun triggers (full body override). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation")
	TObjectPtr<UAnimMontage> GuardBreakMontage;

	/** Played on each blocked hit (1-shot, plays over guard BS pose). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Animation")
	TObjectPtr<UAnimMontage> GuardHitMontage;

	// --- Locomotion ---

	/** Speed-only blendspace used in default (Orient-to-Movement) mode. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Locomotion")
	TObjectPtr<UBlendSpace> LocomotionBS;

	/** Direction + Speed blendspace used while guarding. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Locomotion")
	TObjectPtr<UBlendSpace> GuardLocomotionBS;

	// --- Combo ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Combo")
	int32 MaxComboCount = 4;

	/** Section names in play order: Section_1, Section_2 ... */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Combo")
	TArray<FName> ComboSectionNames;
	
};
