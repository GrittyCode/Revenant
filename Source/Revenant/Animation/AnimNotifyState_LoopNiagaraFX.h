#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Animation/RVFXEntry.h"
#include "AnimNotifyState_LoopNiagaraFX.generated.h"

class UNiagaraComponent;

// Sustains Niagara systems for the duration of the notify window.
// NotifyBegin: spawns systems (bAutoDestroy = false) + plays optional SFX once per entry.
// NotifyEnd:   deactivates and destroys all spawned components.
UCLASS(meta = (DisplayName = "Loop Niagara FX"))
class REVENANT_API UAnimNotifyState_LoopNiagaraFX : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override { return TEXT("LoopNiagaraFX"); }

	UPROPERTY(EditAnywhere, Category = "RV|FX")
	TArray<FRVNiagaraFXEntry> FXList;

private:
	UPROPERTY()
	TArray<TObjectPtr<UNiagaraComponent>> ActiveNCs;
};