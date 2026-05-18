#pragma once

#include "CoreMinimal.h"
#include "Data/RVBossDataAsset.h"
#include "RVSevarogDataAsset.generated.h"

class UAnimMontage;

UCLASS()
class REVENANT_API URVSevarogDataAsset : public URVBossDataAsset
{
	GENERATED_BODY()

public:
	//--- Soul Siphon ---------------------------------------------------------
	// Sevarog channels Soul_Siphon and heals if not interrupted by the player.
	// No hit judgment — pure self-heal on montage completion.

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|SoulSiphon")
	TObjectPtr<UAnimMontage> SoulSiphonMontage;

	// HP recovered when Soul_Siphon completes uninterrupted (flat amount).
	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|SoulSiphon",
		meta = (ClampMin = "0.0"))
	float SoulSiphonHealAmount = 200.f;

	//--- Subjugation ---------------------------------------------------------
	// Sevarog summons a ground field at his feet.
	// Field deals damage per tick to players who remain inside.

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Subjugation")
	TObjectPtr<UAnimMontage> SubjugationMontage;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Subjugation",
		meta = (ClampMin = "0.0"))
	float GroundFieldDuration = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Subjugation",
		meta = (ClampMin = "0.0"))
	float GroundFieldDamagePerTick = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Subjugation",
		meta = (ClampMin = "0.0"))
	float GroundFieldPoiseDamage = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Boss|Subjugation",
		meta = (ClampMin = "0.0"))
	float GroundFieldTickInterval = 0.5f;
};