#include "Animation/AnimNotifyState_AttackHitCheck.h"
#include "Component/RVCombatStateComponent.h"

void UAnimNotifyState_AttackHitCheck::NotifyBegin(USkeletalMeshComponent* MeshComp,
                                                  UAnimSequenceBase* Animation, float TotalDuration,
                                                  const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	URVCombatStateComponent* CombatComp =
		MeshComp->GetOwner()->FindComponentByClass<URVCombatStateComponent>();
	if (!IsValid(CombatComp)) { return; }

	CachedCombatComps.Add(MeshComp, CombatComp);
	CombatComp->OpenHitWindow();
}

void UAnimNotifyState_AttackHitCheck::NotifyTick(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation, float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	URVCombatStateComponent* CombatComp = CachedCombatComps.FindRef(MeshComp);
	if (!IsValid(CombatComp)) { return; }

	CombatComp->PerformAttackTrace();
}

void UAnimNotifyState_AttackHitCheck::NotifyEnd(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	URVCombatStateComponent* CombatComp = CachedCombatComps.FindRef(MeshComp);
	CachedCombatComps.Remove(MeshComp);

	if (!IsValid(CombatComp)) { return; }
	CombatComp->CloseHitWindow();
}

FString UAnimNotifyState_AttackHitCheck::GetNotifyName_Implementation() const
{
	return FString(TEXT("AttackHitCheck"));
}