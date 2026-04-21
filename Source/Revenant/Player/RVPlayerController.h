// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class UInputMappingContext;

DECLARE_LOG_CATEGORY_EXTERN(LogRVPlayerController, Log, All);

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RVPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class REVENANT_API ARVPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Input", Meta = (AllowPrivateAccess = true))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
};
