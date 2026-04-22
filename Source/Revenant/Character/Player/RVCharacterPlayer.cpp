// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/RVCharacterPlayer.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "Component/RVComboComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Input/RVInputConfig.h"

ARVCharacterPlayer::ARVCharacterPlayer()
{
	// SpringArm — 카메라가 벽에 묻히지 않도록 충돌 처리 내장
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 400.0f;
	SpringArm->bUsePawnControlRotation = true; // 컨트롤러 회전을 암에 적용

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; // 암이 회전 하므로 카메라 고정

	// 컨트롤러 Yaw 만 캐릭터에 전달 (카메라 방향으로 이동)
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
	
}

void ARVCharacterPlayer::BeginPlay()
{
	Super::BeginPlay();
}

void ARVCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!ensureMsgf(IsValid(EnhancedInput), TEXT("[ARVCharacterPlayer] EnhancedInputComponent를 찾을 수 없음")))
	{
		return;
	}

	if (!ensureMsgf(IsValid(InputConfig), TEXT("[ARVCharacterPlayer] InputConfig 가 설정되어 있지 않음.")))
	{
		return;
	}


	// InputConfig에서 각 IA 바인딩
	EnhancedInput->BindAction(InputConfig->MoveAction, ETriggerEvent::Triggered, this, &ARVCharacterPlayer::Move);
	EnhancedInput->BindAction(InputConfig->LookAction, ETriggerEvent::Triggered, this, &ARVCharacterPlayer::Look);
	EnhancedInput->BindAction(InputConfig->JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
	EnhancedInput->BindAction(InputConfig->JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	EnhancedInput->BindAction(InputConfig->AttackAction, ETriggerEvent::Started, this, &ARVCharacterPlayer::HandleAttackInput);
}

void ARVCharacterPlayer::Move(const FInputActionValue& InValue)
{
	// Unreal X : 좌우 Y : 앞뒤 Z : 위아래
	const FVector2D MovementVector = InValue.Get<FVector2D>();

	if (!IsValid(Controller))
	{
		return;
	}

	// 컨트롤러 기준 YAW 기준 방향 계산 (카메라가 바라 보는 방향 기준 이동)
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector FowardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(FowardDirection, MovementVector.X);
	AddMovementInput(RightDirection, MovementVector.Y);
}

void ARVCharacterPlayer::Look(const FInputActionValue& InValue)
{
	const FVector2D LookVector = InValue.Get<FVector2D>();

	AddControllerYawInput(LookVector.X);
	AddControllerPitchInput(LookVector.Y);
}

void ARVCharacterPlayer::HandleAttackInput(const FInputActionValue& InValue)
{
	if (IsValid(ComboComponent))
	{
		ComboComponent->TryStartCombo();
	}
}
