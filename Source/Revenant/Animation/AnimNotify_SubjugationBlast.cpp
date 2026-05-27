#include "Animation/AnimNotify_SubjugationBlast.h"
#include "Character/Enemy/RVSevarogCharacter.h"

void UAnimNotify_SubjugationBlast::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* /*Animation*/,
	const FAnimNotifyEventReference& /*EventReference*/)
{
	if (!IsValid(MeshComp)) { return; }

	ARVSevarogCharacter* Boss = Cast<ARVSevarogCharacter>(MeshComp->GetOwner());
	if (!IsValid(Boss)) { return; }

	Boss->SpawnSubjugationBlast(SwirlsFX, SwirlsSFX);
}