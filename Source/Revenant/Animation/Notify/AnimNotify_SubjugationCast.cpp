#include "Animation/Notify/AnimNotify_SubjugationCast.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "Kismet/GameplayStatics.h"

void UAnimNotify_SubjugationCast::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* /*Animation*/,
	const FAnimNotifyEventReference& /*EventReference*/)
{
	if (!IsValid(MeshComp)) { return; }

	ARVSevarogCharacter* Boss = Cast<ARVSevarogCharacter>(MeshComp->GetOwner());
	if (!IsValid(Boss)) { return; }

	// Compute 3 swirl positions, spawn CastFX at each, store positions on Boss.
	Boss->InitSubjugationLocations(CastFX);

	if (IsValid(CastSFX))
	{
		UGameplayStatics::PlaySoundAtLocation(MeshComp, CastSFX, Boss->GetActorLocation());
	}
}