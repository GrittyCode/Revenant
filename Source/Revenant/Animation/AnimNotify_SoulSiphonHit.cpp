#include "Animation/AnimNotify_SoulSiphonHit.h"
#include "Character/Enemy/RVSevarogCharacter.h"

void UAnimNotify_SoulSiphonHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	ARVSevarogCharacter* Boss = Cast<ARVSevarogCharacter>(MeshComp->GetOwner());
	if (!IsValid(Boss)) { return; }

	Boss->ExecuteSoulSiphonHit();
}