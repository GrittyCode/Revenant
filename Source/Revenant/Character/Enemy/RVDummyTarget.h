#pragma once

#include "CoreMinimal.h"
#include "Character/Base/RVCharacterBase.h"
#include "Engine/DataTable.h"
#include "RVDummyTarget.generated.h"

class URVHitReactionAnimDataAsset;

/**
 * Stationary hit-reaction target for demo / testing.
 * Receives attacks, plays stagger / knockdown / death via URVHitReactionComponent.
 * Does not move, does not attack.
 */
UCLASS()
class REVENANT_API ARVDummyTarget : public ARVCharacterBase
{
	GENERATED_BODY()

public:
	ARVDummyTarget();

protected:
	virtual void InitStats() override;
	virtual URVHitReactionAnimDataAsset* GetHitReactionAnimData() const override;

	// --- Data ----------------------------------------------------------------

	/** Stagger / knockdown / death animations. Assign DA_HitReaction_Sword_A in BP. */
	UPROPERTY(EditDefaultsOnly, Category = "RV|Data")
	TObjectPtr<URVHitReactionAnimDataAsset> HitReactionData;

	/**
	 * Row in DT_DummyStats (FRVCharacterStatRow).
	 * Supplies HP, Poise, StaggerDuration, StaggerThreshold, KnockdownThreshold.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "RV|Data")
	FDataTableRowHandle DummyStatRowHandle;
};