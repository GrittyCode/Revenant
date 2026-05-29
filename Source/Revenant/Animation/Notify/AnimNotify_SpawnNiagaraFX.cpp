#include "Animation/Notify/AnimNotify_SpawnNiagaraFX.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

void UAnimNotify_SpawnNiagaraFX::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* /*Animation*/,
	const FAnimNotifyEventReference& /*EventReference*/)
{
	if (!IsValid(MeshComp)) { return; }

	for (const FRVNiagaraFXEntry& Entry : FXList)
	{
		if (IsValid(Entry.NiagaraFX))
		{
			UNiagaraComponent* NC = UNiagaraFunctionLibrary::SpawnSystemAttached(
				Entry.NiagaraFX, MeshComp, Entry.SocketName,
				Entry.LocationOffset, Entry.RotationOffset,
				EAttachLocation::KeepRelativeOffset, true);

			if (IsValid(NC)) { NC->SetRelativeScale3D(Entry.Scale); }
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