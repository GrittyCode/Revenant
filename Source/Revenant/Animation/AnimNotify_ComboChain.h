#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_ComboChain.generated.h"

/**
 * Placed near the end of each combo montage (after ComboWindow closes).
 * Fires TryChainNextCombo — plays next montage if input was buffered, else end combo.
 */
UCLASS()
class REVENANT_API UAnimNotify_ComboChain : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
						const FAnimNotifyEventReference& EventReference) override;
};