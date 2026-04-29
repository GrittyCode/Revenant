// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/RVPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

DEFINE_LOG_CATEGORY(LogRVPlayerController);

void ARVPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// IMC registration is handled by ARVCharacterPlayer::BeginPlay()

}
