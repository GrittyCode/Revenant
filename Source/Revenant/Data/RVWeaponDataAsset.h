// Source/Revenant/Data/RVWeaponDataAsset.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RVWeaponDataAsset.generated.h"

class UBlendSpace;

UCLASS()
class REVENANT_API URVWeaponDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	// --- Attack ---------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "RV|Attack")
	float AttackDamage = 30.f;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Attack")
	float AttackRange;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Attack")
	float AttackRadius;

	// --- Combo ----------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "RV|Combo")
	int32 MaxComboCount = 4;

	/** Section names in play order: Section_1, Section_2 ... */
	UPROPERTY(EditDefaultsOnly, Category = "RV|Combo")
	TArray<FName> ComboSectionNames;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Combo")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Combo")
	float AttackStaminaCost = 20.f;

	// --- Guard ----------------------------------------------------------------

	/**
	 * Guard Montage — three sections in order:
	 * Begin -> Loop (looping) -> End
	 */
	UPROPERTY(EditDefaultsOnly, Category = "RV|Guard")
	TObjectPtr<UAnimMontage> GuardMontage;

	/** Played when Stamina reaches 0 while guarding. */
	UPROPERTY(EditDefaultsOnly, Category = "RV|Guard")
	TObjectPtr<UAnimMontage> GuardBreakMontage;

	// --- Dodge ----------------------------------------------------------------

	/** Single forward roll montage. Character is rotated to face dodge direction before play. */
	UPROPERTY(EditDefaultsOnly, Category = "RV|Dodge")
	TObjectPtr<UAnimMontage> DodgeMontage;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Dodge")
	float DodgeStaminaCost = 30.f;

	// --- Locomotion -----------------------------------------------------------

	/** Blend Space driven by ABP Blend Space Evaluator. Swap per DataAsset instance for A/B styles. */
	UPROPERTY(EditDefaultsOnly, Category = "RV|Locomotion")
	TObjectPtr<UBlendSpace> LocomotionBS;
};