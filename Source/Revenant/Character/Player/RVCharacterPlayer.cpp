#include "Character/Player/RVCharacterPlayer.h"
#include "Camera/CameraComponent.h"
#include "Component/RVCombatComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Input/RVInputConfig.h"
#include "GameFramework/SpringArmComponent.h"

ARVCharacterPlayer::ARVCharacterPlayer()
{
	PrimaryActorTick.bCanEverTick = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 500.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void ARVCharacterPlayer::BeginPlay()
{
	Super::BeginPlay();

	const APlayerController* PC = Cast<APlayerController>(GetController());
	if (!ensureMsgf(IsValid(PC), TEXT("[%s] PlayerController missing — must be possessed by a player"), *GetName()))
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());

	if (!ensureMsgf(IsValid(Subsystem), TEXT("[%s] EnhancedInputLocalPlayerSubsystem missing"), *GetName()))
	{
		return;
	}

	if (!ensureMsgf(IsValid(DefaultMappingContext), TEXT("[%s] DefaultMappingContext not assigned in BP_RVCharacterPlayer"), *GetName()))
	{
		return;
	}

	Subsystem->AddMappingContext(DefaultMappingContext, 0);
}

void ARVCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* Eic = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!IsValid(Eic) || !IsValid(InputConfig)) { return; }

	// ─── Movement ────────────────────────────────────────────────────────────

	Eic->BindAction(InputConfig->MoveAction, ETriggerEvent::Triggered, this, &ARVCharacterPlayer::InputMove);
	Eic->BindAction(InputConfig->LookAction, ETriggerEvent::Triggered, this, &ARVCharacterPlayer::InputLook);
	Eic->BindAction(InputConfig->JumpAction, ETriggerEvent::Started, this, &ARVCharacterPlayer::InputJump);

	// ─── Combat ───────────────────────────────────────────────────────────────

	Eic->BindAction(InputConfig->AttackAction, ETriggerEvent::Started, this, &ARVCharacterPlayer::InputAttack);

	// Dodge: Triggered fires after the Tap threshold is met (set in IA_Dodge).
	// Started would fire immediately on press — before the tap is confirmed.
	Eic->BindAction(InputConfig->DodgeAction, ETriggerEvent::Triggered, this, &ARVCharacterPlayer::InputDodge);

	// Sprint: Started fires when Hold threshold is met, Completed fires on release
	Eic->BindAction(InputConfig->SprintAction, ETriggerEvent::Triggered , this, &ARVCharacterPlayer::InputSprintStarted);
	Eic->BindAction(InputConfig->SprintAction, ETriggerEvent::Completed, this, &ARVCharacterPlayer::InputSprintCompleted);

	// Guard: Started = RMB pressed, Completed = RMB released
	Eic->BindAction(InputConfig->GuardAction, ETriggerEvent::Started, this, &ARVCharacterPlayer::InputGuardStarted);
	Eic->BindAction(InputConfig->GuardAction, ETriggerEvent::Completed, this, &ARVCharacterPlayer::InputGuardCompleted);
}

// ─── Movement ────────────────────────────────────────────────────────────────

void ARVCharacterPlayer::InputMove(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	const FRotator YawOnly(0.f, GetControlRotation().Yaw, 0.f);

	AddMovementInput(FRotationMatrix(YawOnly).GetUnitAxis(EAxis::X), Axis.X);
	AddMovementInput(FRotationMatrix(YawOnly).GetUnitAxis(EAxis::Y), Axis.Y);
}

void ARVCharacterPlayer::InputLook(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void ARVCharacterPlayer::InputJump(const FInputActionValue& Value)
{
	if (!CombatComponent->CanPerformAction(ERVCombatState::Guarding))
	{
		return;
	}

	Jump();
}

// ─── Combat ──────────────────────────────────────────────────────────────────

void ARVCharacterPlayer::InputAttack(const FInputActionValue& Value)
{
	CombatComponent->TryStartCombo();
}

void ARVCharacterPlayer::InputDodge(const FInputActionValue& Value)
{
	// Use last non-zero movement input for directional dodge.
	// Falls back to character forward when standing still.
	FVector DodgeDir = GetLastMovementInputVector();
	if (DodgeDir.IsNearlyZero())
	{
		DodgeDir = GetActorForwardVector();
	}

	CombatComponent->StartDodge(DodgeDir.GetSafeNormal());
}

void ARVCharacterPlayer::InputSprintStarted(const FInputActionValue& Value)
{
   	CombatComponent->StartSprint();
}

void ARVCharacterPlayer::InputSprintCompleted(const FInputActionValue& Value)
{
	CombatComponent->EndSprint();
}

void ARVCharacterPlayer::InputGuardStarted(const FInputActionValue& Value)
{
	CombatComponent->StartGuard();
}

void ARVCharacterPlayer::InputGuardCompleted(const FInputActionValue& Value)
{
	CombatComponent->EndGuard();
}
