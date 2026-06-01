#include "Animation/Notify/AnimNotify_SubjugationBlast.h"
#include "Character/Enemy/RVSevarogCharacter.h"

void UAnimNotify_SubjugationBlast::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* /*Animation*/,
	const FAnimNotifyEventReference& /*EventReference*/)
{
	ARVSevarogCharacter* Boss = Cast<ARVSevarogCharacter>(MeshComp->GetOwner());
	if (!IsValid(Boss)) { return; }

	Boss->SpawnSubjugationBlast(SwirlsFX, SwirlsSFX);
}