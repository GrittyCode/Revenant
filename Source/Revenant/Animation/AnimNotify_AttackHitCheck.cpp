#include "Animation/AnimNotify_AttackHitCheck.h"
#include "Interface/RVCombatInterface.h"

void UAnimNotify_AttackHitCheck::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
										const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	if (IRVCombatInterface* CombatInterface = Cast<IRVCombatInterface>(MeshComp->GetOwner()))
	{
		CombatInterface->ActivateHitCheck();
	}
}

FString UAnimNotify_AttackHitCheck::GetNotifyName_Implementation() const
{
	return FString(TEXT("AttackHitCheck"));
}