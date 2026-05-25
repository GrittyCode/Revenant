#include "Animation/AnimNotify_SpawnFX.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

void UAnimNotify_SpawnFX::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* /*Animation*/,
    const FAnimNotifyEventReference& /*EventReference*/)
{
    if (!IsValid(MeshComp)) { return; }

    for (const FRVFXEntry& Entry : FXList)
    {
        if (!IsValid(Entry.FX)) { continue; }

        UGameplayStatics::SpawnEmitterAttached(
            Entry.FX,
            MeshComp,
            Entry.SocketName,
            Entry.LocationOffset,
            Entry.RotationOffset,
            Entry.Scale,
            EAttachLocation::KeepRelativeOffset,
            true); // bAutoDestroy — point-in-time FX
    }
}
