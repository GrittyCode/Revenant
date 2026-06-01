#include "Animation/Notify/AnimNotify_ComboChain.h"
#include "Character/Player/RVCharacterPlayer.h"

void UAnimNotify_ComboChain::Notify(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* /*Animation*/, const FAnimNotifyEventReference& /*EventReference*/)
{
	ARVCharacterPlayer* Player = Cast<ARVCharacterPlayer>(MeshComp->GetOwner());
	// Notify is placed on player montages only — null here means incorrect montage assignment.
	ensureMsgf(IsValid(Player),
		TEXT("[AnimNotify_ComboChain] Owner is not ARVCharacterPlayer — check montage assignment"));
	if (!IsValid(Player)) { return; }

	Player->TryChainCombo();
}