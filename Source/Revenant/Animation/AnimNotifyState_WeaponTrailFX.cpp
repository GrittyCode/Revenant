#include "Animation/AnimNotifyState_WeaponTrailFX.h"
#include "Character/Base/RVCharacterBase.h"
#include "Kismet/GameplayStatics.h"

void UAnimNotifyState_WeaponTrailFX::NotifyBegin(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* /*Animation*/, float /*TotalDuration*/,
	const FAnimNotifyEventReference& /*EventReference*/)
{
	if (!IsValid(MeshComp)) { return; }

	ARVCharacterBase* Char = Cast<ARVCharacterBase>(MeshComp->GetOwner());
	if (!Char) { return; }

	Char->ActivateWeaponTrail();

	if (IsValid(TrailSFX))
	{
		UGameplayStatics::PlaySoundAtLocation(MeshComp, TrailSFX, Char->GetActorLocation());
	}
}

void UAnimNotifyState_WeaponTrailFX::NotifyEnd(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* /*Animation*/,
	const FAnimNotifyEventReference& /*EventReference*/)
{
	if (!IsValid(MeshComp)) { return; }

	if (ARVCharacterBase* Char = Cast<ARVCharacterBase>(MeshComp->GetOwner()))
	{
		Char->DeactivateWeaponTrail();
	}
}