#include "Animation/AnimNotifyState_LoopFX.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

void UAnimNotifyState_LoopFX::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* /*Animation*/,
	float /*TotalDuration*/, const FAnimNotifyEventReference& /*EventReference*/)
{
	if (!IsValid(MeshComp)) { return; }

	for (const FRVFXEntry& Entry : FXList)
	{
		// ---- Niagara (priority) ----
		if (IsValid(Entry.NiagaraFX))
		{
			UNiagaraComponent* NC = UNiagaraFunctionLibrary::SpawnSystemAttached(
				Entry.NiagaraFX, MeshComp, Entry.SocketName,
				Entry.LocationOffset, Entry.RotationOffset,
				EAttachLocation::KeepRelativeOffset, false);

			if (IsValid(NC))
			{
				NC->SetRelativeScale3D(Entry.Scale);
				ActiveNCs.Add(NC);
			}
			continue;
		}

		// ---- Cascade fallback ----
		if (IsValid(Entry.FX))
		{
			UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAttached(
				Entry.FX, MeshComp, Entry.SocketName,
				Entry.LocationOffset, Entry.RotationOffset, Entry.Scale,
				EAttachLocation::KeepRelativeOffset, false);

			if (IsValid(PSC)) { ActivePSCs.Add(PSC); }
		}
	}
}

void UAnimNotifyState_LoopFX::NotifyEnd(USkeletalMeshComponent* /*MeshComp*/, UAnimSequenceBase* /*Animation*/,
	const FAnimNotifyEventReference& /*EventReference*/)
{
	for (UNiagaraComponent* NC : ActiveNCs)
	{
		if (IsValid(NC)) { NC->DeactivateImmediate(); NC->DestroyComponent(); }
	}
	ActiveNCs.Empty();

	for (UParticleSystemComponent* PSC : ActivePSCs)
	{
		if (IsValid(PSC)) { PSC->DeactivateSystem(); PSC->DestroyComponent(); }
	}
	ActivePSCs.Empty();
}