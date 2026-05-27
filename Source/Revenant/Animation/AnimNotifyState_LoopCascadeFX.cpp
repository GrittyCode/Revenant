#include "Animation/AnimNotifyState_LoopCascadeFX.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

void UAnimNotifyState_LoopCascadeFX::NotifyBegin(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* /*Animation*/, float /*TotalDuration*/,
	const FAnimNotifyEventReference& /*EventReference*/)
{
	if (!IsValid(MeshComp)) { return; }

	for (const FRVCascadeFXEntry& Entry : FXList)
	{
		if (IsValid(Entry.FX))
		{
			UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAttached(
				Entry.FX, MeshComp, Entry.SocketName,
				Entry.LocationOffset, Entry.RotationOffset, Entry.Scale,
				EAttachLocation::KeepRelativeOffset, false);

			if (IsValid(PSC)) { ActivePSCs.Add(PSC); }
		}

		// SFX plays once at loop start, independent of FX validity.
		if (IsValid(Entry.SFX))
		{
			const FVector Loc = (Entry.SocketName != NAME_None)
				? MeshComp->GetSocketLocation(Entry.SocketName)
				: MeshComp->GetComponentLocation();
			UGameplayStatics::PlaySoundAtLocation(MeshComp, Entry.SFX, Loc);
		}
	}
}

void UAnimNotifyState_LoopCascadeFX::NotifyEnd(USkeletalMeshComponent* /*MeshComp*/,
	UAnimSequenceBase* /*Animation*/, const FAnimNotifyEventReference& /*EventReference*/)
{
	for (UParticleSystemComponent* PSC : ActivePSCs)
	{
		if (IsValid(PSC)) { PSC->DeactivateSystem(); PSC->DestroyComponent(); }
	}
	ActivePSCs.Empty();
}