#include "Animation/Notify/AnimNotify_PlaySFX.h"
#include "Kismet/GameplayStatics.h"

void UAnimNotify_PlaySFX::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* /*Animation*/,
	const FAnimNotifyEventReference& /*EventReference*/)
{
	if (!IsValid(SFX)) { return; }

	const FVector Loc = (SocketName != NAME_None)
		? MeshComp->GetSocketLocation(SocketName)
		: MeshComp->GetComponentLocation();

	UGameplayStatics::PlaySoundAtLocation(MeshComp, SFX, Loc, VolumeMultiplier);
}