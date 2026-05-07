#include "Animation/AnimNotifyState_DodgeIFrame.h"
#include "Component/RVCombatComponent.h"

void UAnimNotifyState_DodgeIFrame::NotifyBegin(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	URVCombatComponent* CombatComp =
		MeshComp->GetOwner()->FindComponentByClass<URVCombatComponent>();
	if (!IsValid(CombatComp)) { return; }

	CachedCombatComps.Add(MeshComp, CombatComp);
	CombatComp->SetDodgeIFrame(true);
}

void UAnimNotifyState_DodgeIFrame::NotifyEnd(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	URVCombatComponent* CombatComp = CachedCombatComps.FindRef(MeshComp);
	CachedCombatComps.Remove(MeshComp);

	if (!IsValid(CombatComp)) { return; }
	CombatComp->SetDodgeIFrame(false);
}

FString UAnimNotifyState_DodgeIFrame::GetNotifyName_Implementation() const
{
	return FString(TEXT("DodgeIFrame"));
}