// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify_ComboWindow.h"

#include "Component/RVComboComponent.h"
#include "GameFramework/Character.h"

void UAnimNotify_ComboWindow::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	ACharacter* OwnerCharacter = Cast<ACharacter>(MeshComp->GetOwner());
	
	if (!IsValid(OwnerCharacter))
	{
		return;
	}
	
	URVComboComponent* ComboComp = OwnerCharacter->FindComponentByClass<URVComboComponent>();
	
	if (!IsValid(ComboComp))
	{
		return;
	}
	
	ComboComp->TryAdvancedCombo();
}

FString UAnimNotify_ComboWindow::GetNotifyName_Implementation() const
{
	return Super::GetNotifyName_Implementation();
}
