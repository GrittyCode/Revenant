#include "Animation/AnimNotify_ComboChain.h"
#include "Component/RVComboComponent.h"

void UAnimNotify_ComboChain::Notify(USkeletalMeshComponent* MeshComp,
									UAnimSequenceBase* Animation,
									const FAnimNotifyEventReference& EventReference)
{
	AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner)) { return; }

	URVComboComponent* ComboComp = Owner->FindComponentByClass<URVComboComponent>();
	if (!IsValid(ComboComp)) { return; }

	ComboComp->TryChainNextCombo();
}