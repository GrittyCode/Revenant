#include "Animation/AnimNotifyState_LoopNiagaraFX.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

static void CleanupNiagaraComponents(TArray<TObjectPtr<UNiagaraComponent>>& InNCs)
{
	for (UNiagaraComponent* NC : InNCs)
	{
		if (IsValid(NC)) { NC->DeactivateImmediate(); NC->DestroyComponent(); }
	}
	InNCs.Empty();
}

void UAnimNotifyState_LoopNiagaraFX::NotifyBegin(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* /*Animation*/, float /*TotalDuration*/,
	const FAnimNotifyEventReference& /*EventReference*/)
{
	if (!IsValid(MeshComp)) { return; }

	// Clean up leftovers from a previous interrupted play before spawning new ones.
	CleanupNiagaraComponents(ActiveNCs);

	for (const FRVNiagaraFXEntry& Entry : FXList)
	{
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
		}

		// SFX plays once at loop start, independent of NiagaraFX validity.
		if (IsValid(Entry.SFX))
		{
			const FVector Loc = (Entry.SocketName != NAME_None)
				? MeshComp->GetSocketLocation(Entry.SocketName)
				: MeshComp->GetComponentLocation();
			UGameplayStatics::PlaySoundAtLocation(MeshComp, Entry.SFX, Loc);
		}
	}
}

void UAnimNotifyState_LoopNiagaraFX::NotifyEnd(USkeletalMeshComponent* /*MeshComp*/,
	UAnimSequenceBase* /*Animation*/, const FAnimNotifyEventReference& /*EventReference*/)
{
	CleanupNiagaraComponents(ActiveNCs);
}
