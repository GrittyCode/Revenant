#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_WeaponTrailFX.generated.h"

class USoundBase;


UCLASS(meta = (DisplayName = "Weapon Trail FX"))
class REVENANT_API UAnimNotifyState_WeaponTrailFX : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override { return TEXT("WeaponTrailFX"); }

	// Swing sound played once when the trail activates.
	// Assign a different sound per attack in the montage for variation.
	UPROPERTY(EditAnywhere, Category = "RV|FX")
	TObjectPtr<USoundBase> TrailSFX;
};