#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_DodgeIFrame.generated.h"

class URVCombatComponent;

UCLASS()
class REVENANT_API UAnimNotifyState_DodgeIFrame : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

private:
	UPROPERTY()
	// Keyed by MeshComp — safe for multiple characters playing the same montage simultaneously.
	TMap<USkeletalMeshComponent*, URVCombatComponent*> CachedCombatComps;
};