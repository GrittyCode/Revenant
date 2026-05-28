#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Animation/RVFXEntry.h"
#include "AnimNotifyState_LoopCascadeFX.generated.h"

class UParticleSystemComponent;

// Sustains Cascade particle systems for the duration of the notify window.
// NotifyBegin: cleans up any leftover components from a previous interrupted play, then spawns fresh.
// NotifyEnd:   deactivates and destroys all spawned components.
UCLASS(meta = (DisplayName = "Loop Cascade FX"))
class REVENANT_API UAnimNotifyState_LoopCascadeFX : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override { return TEXT("LoopCascadeFX"); }

	UPROPERTY(EditAnywhere, Category = "RV|FX")
	TArray<FRVCascadeFXEntry> FXList;

private:
	// Tracks spawned components for this notify window.
	// Cleaned up at the start of NotifyBegin to handle interrupted montages.
	UPROPERTY()
	TArray<TObjectPtr<UParticleSystemComponent>> ActivePSCs;
};
