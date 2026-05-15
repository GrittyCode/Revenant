#include "Animation/AnimNotifyState_ComboWindow.h"
#include "Component/RVComboComponent.h"

void UAnimNotifyState_ComboWindow::NotifyBegin(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner)) { return; }

	URVComboComponent* ComboComp = Owner->FindComponentByClass<URVComboComponent>();
	if (!IsValid(ComboComp)) { return; }

	CachedComboComps.Add(MeshComp, ComboComp);
	ComboComp->OpenComboWindow();
}

void UAnimNotifyState_ComboWindow::NotifyEnd(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	URVComboComponent* ComboComp = CachedComboComps.FindRef(MeshComp);
	CachedComboComps.Remove(MeshComp);

	if (!IsValid(ComboComp)) { return; }
	ComboComp->CloseComboWindow();
}

FString UAnimNotifyState_ComboWindow::GetNotifyName_Implementation() const
{
	return FString(TEXT("ComboWindow"));
}