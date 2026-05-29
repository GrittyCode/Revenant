#include "Animation/Notify/AnimNotify_SpawnCascadeFX.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

void UAnimNotify_SpawnCascadeFX::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* /*Animation*/,
	const FAnimNotifyEventReference& /*EventReference*/)
{
	if (!IsValid(MeshComp)) { return; }

	for (const FRVCascadeFXEntry& Entry : FXList)
	{
		if (IsValid(Entry.FX))
		{
			UGameplayStatics::SpawnEmitterAttached(
				Entry.FX, MeshComp, Entry.SocketName,
				Entry.LocationOffset, Entry.RotationOffset, Entry.Scale,
				EAttachLocation::KeepRelativeOffset, true);
		}

		if (IsValid(Entry.SFX))
		{
			const FVector Loc = (Entry.SocketName != NAME_None)
				? MeshComp->GetSocketLocation(Entry.SocketName)
				: MeshComp->GetComponentLocation();
			UGameplayStatics::PlaySoundAtLocation(MeshComp, Entry.SFX, Loc);
		}
	}
}