#include "Animation/AnimNotify_DodgeIFrame.h"

void UAnimNotify_DodgeIFrame::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
									 const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

}

FString UAnimNotify_DodgeIFrame::GetNotifyName_Implementation() const
{
	// Return a meaningful name instead of forwarding to Super (which returns the class name verbatim).
	return bActivate ? TEXT("DodgeIFrame_On") : TEXT("DodgeIFrame_Off");
}