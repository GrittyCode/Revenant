#include "Animation/AnimNotify_ComboWindow.h"
#include "Component/RVComboComponent.h"

void UAnimNotify_ComboWindow::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
									 const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	URVComboComponent* ComboComp = MeshComp->GetOwner()->FindComponentByClass<URVComboComponent>();
	if (!IsValid(ComboComp))
	{
		return;
	}

	ComboComp->TryAdvanceCombo();
}

FString UAnimNotify_ComboWindow::GetNotifyName_Implementation() const
{
	return TEXT("ComboWindow");
}