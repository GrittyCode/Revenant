#pragma once

#include "CoreMinimal.h"
#include "Character/Base/RVCharacterBase.h"
#include "RVDummyTarget.generated.h"

UCLASS()
class REVENANT_API ARVDummyTarget : public ARVCharacterBase
{
	GENERATED_BODY()

public:
	ARVDummyTarget();

	virtual void Tick(float DeltaTime) override;

	/** Overrides base to also trigger debug label on hit received. */
	virtual bool ApplyDamage(const FRVHitInfo& InHitInfo) override;

protected:
	virtual void BeginPlay() override;

private:
	// --- Periodic Damage (simulates weapon collision result) -----------------

	/** If true, periodically calls ApplyDamage on the player pawn. */
	UPROPERTY(EditInstanceOnly, Category = "RV|Test")
	uint8 bDealPeriodicDamage : 1;

	UPROPERTY(EditInstanceOnly, Category = "RV|Test")
	float DealDamageInterval = 1.5f;

	UPROPERTY(EditInstanceOnly, Category = "RV|Test")
	float DealDamageAmount = 30.f;

	/**
	 * Poise damage dealt in periodic test hits.
	 * Tunable per instance to test stagger / groggy thresholds in the editor.
	 */
	UPROPERTY(EditInstanceOnly, Category = "RV|Test")
	float DealPoiseDamage = 30.f;

	// --- Debug Display -------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "RV|Test")
	float HitDisplayDuration = 0.3f;

	// Runtime state
	float TimeUntilNextDamage = 0.f;
	float HitDisplayTimer     = 0.f;
	float LastReceivedDamage  = 0.f;

	TWeakObjectPtr<AActor> CachedPlayer;

	void DealDamageToPlayer();
};