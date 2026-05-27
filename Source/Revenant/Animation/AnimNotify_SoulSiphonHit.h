#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotify_SpawnCascadeFX.h"
#include "AnimNotify_SoulSiphonHit.generated.h"


UCLASS(meta = (DisplayName = "Soul Siphon Hit"))
class REVENANT_API UAnimNotify_SoulSiphonHit : public UAnimNotify_SpawnCascadeFX
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override { return TEXT("SoulSiphonHit"); }
};