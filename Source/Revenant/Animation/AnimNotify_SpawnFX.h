#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/RVFXEntry.h"
#include "AnimNotify_SpawnFX.generated.h"

UCLASS(meta = (DisplayName = "Spawn FX"))
class REVENANT_API UAnimNotify_SpawnFX : public UAnimNotify
{
    GENERATED_BODY()

public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;

    virtual FString GetNotifyName_Implementation() const override
    {
        return TEXT("SpawnFX");
    }

    UPROPERTY(EditAnywhere, Category = "RV|FX")
    TArray<FRVFXEntry> FXList;
};
