// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify_AttackHitCheck.h"

#include "Interface/RVCombatInterface.h"

void UAnimNotify_AttackHitCheck::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	if (!IsValid(MeshComp))
	{
		return;
	}
	
	// Owner actor must implement IRVCombatInterface.
	AActor* OwnerActor = MeshComp->GetOwner();
	if (IRVCombatInterface* CombatInterface = Cast<IRVCombatInterface>(OwnerActor))
	{
		CombatInterface->Execute_ActivateHitCheck(OwnerActor);
	}
}

FString UAnimNotify_AttackHitCheck::GetNotifyName_Implementation() const
{
	return FString(TEXT("AttackHitCheck"));
}
