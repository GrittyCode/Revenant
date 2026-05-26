#include "Animation/AnimNotify_SpawnFX.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

void UAnimNotify_SpawnFX::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* /*Animation*/,
	const FAnimNotifyEventReference& /*EventReference*/)
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
				EAttachLocation::KeepRelativeOffset, true);

			if (IsValid(NC)) { NC->SetRelativeScale3D(Entry.Scale); }
		}
		// ---- Cascade fallback ----
		else if (IsValid(Entry.FX))
		{
			UGameplayStatics::SpawnEmitterAttached(
				Entry.FX, MeshComp, Entry.SocketName,
				Entry.LocationOffset, Entry.RotationOffset, Entry.Scale,
				EAttachLocation::KeepRelativeOffset, true);
		}

		// ---- SFX (independent of VFX) ----
		if (IsValid(Entry.SFX))
		{
			const FVector Loc = (Entry.SocketName != NAME_None)
				? MeshComp->GetSocketLocation(Entry.SocketName)
				: MeshComp->GetComponentLocation();
			UGameplayStatics::PlaySoundAtLocation(MeshComp, Entry.SFX, Loc);
		}
	}
}