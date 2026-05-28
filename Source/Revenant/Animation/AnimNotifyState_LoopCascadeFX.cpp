#include "Animation/AnimNotifyState_LoopCascadeFX.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

static void CleanupParticleComponents(TArray<TObjectPtr<UParticleSystemComponent>>& InPSCs)
{
	for (UParticleSystemComponent* PSC : InPSCs)
	{
		if (IsValid(PSC)) { PSC->DeactivateSystem(); PSC->DestroyComponent(); }
	}
	InPSCs.Empty();
}

void UAnimNotifyState_LoopCascadeFX::NotifyBegin(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* /*Animation*/, float /*TotalDuration*/,
	const FAnimNotifyEventReference& /*EventReference*/)
{
	if (!IsValid(MeshComp)) { return; }

	// Clean up leftovers from a previous interrupted play before spawning new ones.
	CleanupParticleComponents(ActivePSCs);

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
	CleanupParticleComponents(ActivePSCs);
}
