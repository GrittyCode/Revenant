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

	/** Speed-only blend space used in default (Orient-to-Movement) mode. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Locomotion")
	TObjectPtr<UBlendSpace> LocomotionBS;

	/**
	 * Direction + Speed blend space used in lock-on (strafe) mode.
	 * bOrientRotationToMovement is off while locked on.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Locomotion")
	TObjectPtr<UBlendSpace> LockOnLocomotionBS;

	/**
	 * Speed-only guard blend space used while guarding without lock-on.
	 * Uses forward loop only; character rotates via bOrientRotationToMovement.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Locomotion")
	TObjectPtr<UBlendSpace> GuardLocomotionBS;

	/**
	 * Direction + Speed blend space used while guarding with lock-on active.
	 * Full 6-direction strafe; bOrientRotationToMovement is off in this mode.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Locomotion")
	TObjectPtr<UBlendSpace> GuardLocomotionBS_LockOn;

	// --- Combo ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Combo")
	int32 MaxComboCount = 4;

	/** Section names in play order: Section_1, Section_2 ... */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Combo")
	TArray<FName> ComboSectionNames;
};