#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Animation/RVFXEntry.h"
#include "AnimNotifyState_LoopFX.generated.h"

class UParticleSystemComponent;

UCLASS(meta = (DisplayName = "Loop FX"))
class REVENANT_API UAnimNotifyState_LoopFX : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;

    virtual FString GetNotifyName_Implementation() const override
    {
        return TEXT("LoopFX");
    }

    UPROPERTY(EditAnywhere, Category = "RV|FX")
    TArray<FRVFXEntry> FXList;

private:
    // Tracks spawned PSCs per mesh — one notify state instance is shared across all callers.
    UPROPERTY()
	TArray<UParticleSystemComponent*> ActivePSCs;
};
