#include "Animation/AnimNotifyState_AttackHitCheck.h"
#include "Character/Base/RVCharacterBase.h"

void UAnimNotifyState_AttackHitCheck::NotifyBegin(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	ARVCharacterBase* OwnerChar = Cast<ARVCharacterBase>(MeshComp->GetOwner());
	if (!IsValid(OwnerChar)) { return; }

	CachedOwners.Add(MeshComp, OwnerChar);
	OwnerChar->OpenAttackHitWindow();
}

void UAnimNotifyState_AttackHitCheck::NotifyTick(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation, float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	ARVCharacterBase* OwnerChar = CachedOwners.FindRef(MeshComp);
	if (!IsValid(OwnerChar)) { return; }

	OwnerChar->ActivateHitCheck();
}

void UAnimNotifyState_AttackHitCheck::NotifyEnd(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	ARVCharacterBase* OwnerChar = CachedOwners.FindRef(MeshComp);
	CachedOwners.Remove(MeshComp);

	if (!IsValid(OwnerChar)) { return; }
	OwnerChar->CloseAttackHitWindow();
}

FString UAnimNotifyState_AttackHitCheck::GetNotifyName_Implementation() const
{
	return FString(TEXT("AttackHitCheck"));
}