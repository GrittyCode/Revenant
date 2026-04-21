// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/RVPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

DEFINE_LOG_CATEGORY(LogRVPlayerController);

void ARVPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// Local Player 기준으로 Enhanced Input SubSystem 에 IMC 등록
	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	
	if (!ensureMsgf(IsValid(Subsystem), TEXT("[ARVPlayerController] EnhancedInputLocalPlayerSubsystem을 찾을 수 없음")))
	{
		return;
	}

	if (!ensureMsgf(IsValid(DefaultMappingContext), TEXT("[ARVPlayerController] DefaultMappingContext가 설정되지 않음 — BP_RVPlayerController에서 IMC를 할당해야 함")))
	{
		return;
	}
	
	// Priority 0 - 기본 이동 레이어, 나중에 전투 레이어가 위에 쌓임
	Subsystem->AddMappingContext(DefaultMappingContext, 0);

}
