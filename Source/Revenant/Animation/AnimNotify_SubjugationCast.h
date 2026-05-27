#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_SubjugationCast.generated.h"

class UParticleSystem;
class USoundBase;


UCLASS(meta = (DisplayName = "Subjugation Cast"))
class REVENANT_API UAnimNotify_SubjugationCast : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override { return TEXT("SubjugationCast"); }

	// Warning FX spawned at each of the 3 computed swirl positions (e.g. P_Sub_Cast).
	UPROPERTY(EditAnywhere, Category = "RV|FX")
	TObjectPtr<UParticleSystem> CastFX;

	// Optional SFX played once at boss location when the cast begins.
	UPROPERTY(EditAnywhere, Category = "RV|SFX")
	TObjectPtr<USoundBase> CastSFX;
};