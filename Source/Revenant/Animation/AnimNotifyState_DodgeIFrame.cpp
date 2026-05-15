#include "Animation/AnimNotifyState_DodgeIFrame.h"
#include "Component/RVDodgeComponent.h"

void UAnimNotifyState_DodgeIFrame::NotifyBegin(USkeletalMeshComponent* MeshComp,
											   UAnimSequenceBase* Animation, float TotalDuration,
											   const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner)) { return; }

	URVDodgeComponent* DodgeComp = Owner->FindComponentByClass<URVDodgeComponent>();
	if (!IsValid(DodgeComp)) { return; }

	DodgeComp->SetDodgeIFrame(true);
}

void UAnimNotifyState_DodgeIFrame::NotifyEnd(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner)) { return; }

	URVDodgeComponent* DodgeComp = Owner->FindComponentByClass<URVDodgeComponent>();
	if (!IsValid(DodgeComp)) { return; }

	DodgeComp->SetDodgeIFrame(false);
}

FString UAnimNotifyState_DodgeIFrame::GetNotifyName_Implementation() const
{
	return FString(TEXT("DodgeIFrame"));
}