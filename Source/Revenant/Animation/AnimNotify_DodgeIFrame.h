// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_DodgeIFrame.generated.h"

/**
 * 
 */
UCLASS()
class REVENANT_API UAnimNotify_DodgeIFrame : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	                    const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;
	
	
	UPROPERTY(EditAnywhere, Category = "RV|Animation")
	bool bActivate = true;
};
