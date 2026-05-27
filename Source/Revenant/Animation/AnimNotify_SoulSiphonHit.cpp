#include "Animation/AnimNotify_SoulSiphonHit.h"
#include "Character/Enemy/RVSevarogCharacter.h"

void UAnimNotify_SoulSiphonHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	// FXList Cascade FX + SFX spawn (cast FX assigned in montage editor).
	Super::Notify(MeshComp, Animation, EventReference);

	ARVSevarogCharacter* Boss = Cast<ARVSevarogCharacter>(MeshComp->GetOwner());
	if (!IsValid(Boss)) { return; }

	// Damage + impact FX at computed hit center + debug capsule.
	Boss->ExecuteSoulSiphonHit();
}