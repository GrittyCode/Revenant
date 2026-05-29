#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/Notify/RVFXEntry.h"
#include "AnimNotify_SoulSiphonHit.generated.h"


UCLASS(meta = (DisplayName = "Soul Siphon Hit"))
class REVENANT_API UAnimNotify_SoulSiphonHit : public UAnimNotify
{
    GENERATED_BODY()

public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;

    virtual FString GetNotifyName_Implementation() const override { return TEXT("SoulSiphonHit"); }

    UPROPERTY(EditAnywhere, Category = "RV|FX")
    TArray<FRVCascadeFXEntry> FXList;
};
