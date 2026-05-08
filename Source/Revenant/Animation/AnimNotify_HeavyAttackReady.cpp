#include "Animation/AnimNotify_HeavyAttackReady.h"
#include "Component/RVCombatComponent.h"

void UAnimNotify_HeavyAttackReady::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	URVCombatComponent* CombatComp =
	 MeshComp->GetOwner()->FindComponentByClass<URVCombatComponent>();
	if (!IsValid(CombatComp)) { return; }

	CombatComp->SetHeavyAttackReady(true);
}

FString UAnimNotify_HeavyAttackReady::GetNotifyName_Implementation() const
{
	return FString(TEXT("HeavyAttackReady"));
}
