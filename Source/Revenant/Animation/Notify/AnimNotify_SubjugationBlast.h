#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_SubjugationBlast.generated.h"

class UParticleSystem;
class USoundBase;

UCLASS(meta = (DisplayName = "Subjugation Blast"))
class REVENANT_API UAnimNotify_SubjugationBlast : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override { return TEXT("SubjugationBlast"); }

	// VFX spawned at the 3 stored swirl positions.
	UPROPERTY(EditAnywhere, Category = "RV|FX")
	TObjectPtr<UParticleSystem> SwirlsFX;

	// SFX played at each swirl location when damage is applied.
	UPROPERTY(EditAnywhere, Category = "RV|SFX")
	TObjectPtr<USoundBase> SwirlsSFX;

	// Seconds after SwirlsFX spawns before damage and SFX are applied.
	// Tune to match the moment the FX visually hits the ground.
	UPROPERTY(EditAnywhere, Category = "RV|Combat", meta = (ClampMin = "0.0"))
	float DamageDelay = 0.5f;
};