#include "Animation/Notify/AnimNotify_BossComboChain.h"
#include "Character/Enemy/RVSevarogCharacter.h"

void UAnimNotify_BossComboChain::Notify(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* /*Animation*/, const FAnimNotifyEventReference& /*EventReference*/)
{
	ARVSevarogCharacter* Boss = Cast<ARVSevarogCharacter>(MeshComp->GetOwner());

	ensureMsgf(IsValid(Boss),
		TEXT("[AnimNotify_BossComboChain] Owner is not ARVSevarogCharacter — check montage assignment"));
	if (!IsValid(Boss)) { return; }

	Boss->TryChainCombo();
}