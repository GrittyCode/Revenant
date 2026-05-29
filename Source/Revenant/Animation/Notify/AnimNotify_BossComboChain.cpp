#include "Animation/Notify/AnimNotify_BossComboChain.h"
#include "Character/Enemy/RVSevarogCharacter.h"

void UAnimNotify_BossComboChain::Notify(USkeletalMeshComponent* MeshComp,
										UAnimSequenceBase* Animation,
										const FAnimNotifyEventReference& EventReference)
{
	ARVSevarogCharacter* Boss = Cast<ARVSevarogCharacter>(MeshComp->GetOwner());
	if (!IsValid(Boss)) { return; }

	Boss->TryChainCombo();
}