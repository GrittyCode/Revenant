// Source/Revenant/Animation/AnimNotify_DodgeIFrame.h
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_DodgeIFrame.generated.h"

UCLASS()
class REVENANT_API UAnimNotify_DodgeIFrame : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp,
						UAnimSequenceBase*       Animation,
						const FAnimNotifyEventReference& EventReference) override;

	/** True = open i-frame window, False = close it. Set per notify instance in Montage editor. */
	UPROPERTY(EditAnywhere, Category = "RV|Dodge")
	bool bActivate = true;
};