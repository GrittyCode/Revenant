#include "Animation/AnimNotifyState_LoopFX.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

void UAnimNotifyState_LoopFX::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* /*Animation*/,
	float /*TotalDuration*/, const FAnimNotifyEventReference& /*EventReference*/)
{
	if (!IsValid(MeshComp)) { return; }

	for (const FRVFXEntry& Entry : FXList)
	{
		if (!IsValid(Entry.FX)) { continue; }

		UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAttached(
			Entry.FX,
			MeshComp,
			Entry.SocketName,
			Entry.LocationOffset,
			Entry.RotationOffset,
			Entry.Scale,
			EAttachLocation::KeepRelativeOffset,
			false); // bAutoDestroy = false — lifetime controlled by NotifyEnd

		if (IsValid(PSC))
		{
			ActivePSCs.Add(PSC);
		}
	}
}

void UAnimNotifyState_LoopFX::NotifyEnd(USkeletalMeshComponent* /*MeshComp*/, UAnimSequenceBase* /*Animation*/,
	const FAnimNotifyEventReference& /*EventReference*/)
{
	for (UParticleSystemComponent* PSC : ActivePSCs)
	{
		if (IsValid(PSC))
		{
			PSC->DeactivateSystem();
			PSC->DestroyComponent();
		}
	}
	ActivePSCs.Empty();
}