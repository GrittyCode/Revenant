#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "RVCombatDataAsset.generated.h"

struct FRVWeaponStatRow;
class UBlendSpace;
class UAnimMontage;

UCLASS(BlueprintType)
class REVENANT_API URVCombatDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	//--- Base Stats ----------------------------------------------------------
	// Points to a row in DT_WeaponStats.
	// Final hit values = WeaponStat.Base* x AttackStatRow.Multiplier.

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Combat")
	FDataTableRowHandle WeaponStatRowHandle;

	const FRVWeaponStatRow* GetWeaponStatRow() const;

	//--- Hit Reaction Animations ---------------------------------------------

	// Direction axis: -180 to 180. ABP samples at StaggerDirection during HitReaction state.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|HitReaction")
	TObjectPtr<UBlendSpace> StaggerBlendSpace;

	// Transitions to GetUpMontage on blend-out.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|HitReaction")
	TObjectPtr<UAnimMontage> KnockdownMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|HitReaction")
	TObjectPtr<UAnimMontage> GetUpMontage;
};