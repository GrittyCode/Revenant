#include "Animation/AnimNotify_ComboChain.h"
#include "Component/RVComboComponent.h"

void UAnimNotify_ComboChain::Notify(USkeletalMeshComponent* MeshComp,
									 UAnimSequenceBase* Animation,
									 const FAnimNotifyEventReference& EventReference)
{
	URVComboComponent* ComboComp = MeshComp->GetOwner()->FindComponentByClass<URVComboComponent>();
	if (!IsValid(ComboComp)) { return; }

	ComboComp->TryChainNextCombo();
}