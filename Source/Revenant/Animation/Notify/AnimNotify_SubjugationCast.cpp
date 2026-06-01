#include "Animation/Notify/AnimNotify_SubjugationCast.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "Kismet/GameplayStatics.h"

void UAnimNotify_SubjugationCast::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* /*Animation*/,
	const FAnimNotifyEventReference& /*EventReference*/)
{
	ARVSevarogCharacter* Boss = Cast<ARVSevarogCharacter>(MeshComp->GetOwner());
	if (!IsValid(Boss)) { return; }

	Boss->InitSubjugationLocations(CastFX);

	if (IsValid(CastSFX))
	{
		UGameplayStatics::PlaySoundAtLocation(MeshComp, CastSFX, Boss->GetActorLocation());
	}
}