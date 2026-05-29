#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/Notify/RVFXEntry.h"
#include "AnimNotify_SpawnCascadeFX.generated.h"

// Spawns one or more Cascade particle systems attached to a socket on notify fire.
// Optional SFX per entry plays at the socket location.
UCLASS(meta = (DisplayName = "Spawn Cascade FX"))
class REVENANT_API UAnimNotify_SpawnCascadeFX : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override { return TEXT("SpawnCascadeFX"); }

	UPROPERTY(EditAnywhere, Category = "RV|FX")
	TArray<FRVCascadeFXEntry> FXList;
};