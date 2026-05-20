// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "BTDecorator_SevarogCheckPhase.generated.h"


UCLASS()
class REVENANT_API UBTDecorator_SevarogCheckPhase : public UBTDecorator
{
	GENERATED_BODY()
 
public:
	UBTDecorator_SevarogCheckPhase();
 
	// Minimum phase required for this branch to execute.
	UPROPERTY(EditAnywhere, Category = "RV|Boss")
	ERVBossPhase RequiredPhase = ERVBossPhase::Phase2;
 
protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;
};