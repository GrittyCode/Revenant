#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/Notify/RVFXEntry.h"
#include "AnimNotify_SpawnNiagaraFX.generated.h"

// Spawns one or more Niagara systems attached to a socket on notify fire.
// Optional SFX per entry plays at the socket location.
UCLASS(meta = (DisplayName = "Spawn Niagara FX"))
class REVENANT_API UAnimNotify_SpawnNiagaraFX : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override { return TEXT("SpawnNiagaraFX"); }

	UPROPERTY(EditAnywhere, Category = "RV|FX")
	TArray<FRVNiagaraFXEntry> FXList;
};