// Source/Revenant/Character/Player/RVCharacterPlayer.cpp
#include "Character/Player/RVCharacterPlayer.h"
#include "Input/RVInputConfig.h"
#include "Component/RVCombatComponent.h"
#include "Component/RVComboComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

ARVCharacterPlayer::ARVCharacterPlayer()
{
    PrimaryActorTick.bCanEverTick = false;
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength         = 500.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void ARVCharacterPlayer::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!IsValid(PC)) { return; }

    UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());

    if (IsValid(Subsystem) && IsValid(DefaultMappingContext))
    {
        Subsystem->AddMappingContext(DefaultMappingContext, 0);
    }
}

void ARVCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!IsValid(EIC) || !IsValid(InputConfig)) { return; }

    // ─── Movement ────────────────────────────────────────────────────────────

    EIC->BindAction(InputConfig->MoveAction,   ETriggerEvent::Triggered, this, &ARVCharacterPlayer::InputMove);
    EIC->BindAction(InputConfig->LookAction,   ETriggerEvent::Triggered, this, &ARVCharacterPlayer::InputLook);
    EIC->BindAction(InputConfig->JumpAction,   ETriggerEvent::Started,   this, &ARVCharacterPlayer::InputJump);

    // ─── Combat ───────────────────────────────────────────────────────────────

    EIC->BindAction(InputConfig->AttackAction, ETriggerEvent::Started,   this, &ARVCharacterPlayer::InputAttack);

    // Dodge: Started trigger fires once on tap (threshold set in IA_Dodge)
    EIC->BindAction(InputConfig->DodgeAction,  ETriggerEvent::Started,   this, &ARVCharacterPlayer::InputDodge);

    // Sprint: hold threshold handled in IA_Sprint
    EIC->BindAction(InputConfig->SprintAction, ETriggerEvent::Started,   this, &ARVCharacterPlayer::InputSprintStarted);
    EIC->BindAction(InputConfig->SprintAction, ETriggerEvent::Completed, this, &ARVCharacterPlayer::InputSprintCompleted);

    // Guard: Started = RMB pressed, Completed = RMB released
    EIC->BindAction(InputConfig->GuardAction,  ETriggerEvent::Started,   this, &ARVCharacterPlayer::InputGuardStarted);
    EIC->BindAction(InputConfig->GuardAction,  ETriggerEvent::Completed, this, &ARVCharacterPlayer::InputGuardCompleted);
}

// ─── Movement ────────────────────────────────────────────────────────────────

void ARVCharacterPlayer::InputMove(const FInputActionValue& Value)
{
    const FVector2D Axis = Value.Get<FVector2D>();
    const FRotator  YawOnly(0.f, GetControlRotation().Yaw, 0.f);

    AddMovementInput(FRotationMatrix(YawOnly).GetUnitAxis(EAxis::X), Axis.X);
    AddMovementInput(FRotationMatrix(YawOnly).GetUnitAxis(EAxis::Y), Axis.Y);
}

void ARVCharacterPlayer::InputLook(const FInputActionValue& Value)
{
    const FVector2D Axis = Value.Get<FVector2D>();
    AddControllerYawInput  (Axis.X);
    AddControllerPitchInput(Axis.Y);
}

void ARVCharacterPlayer::InputJump(const FInputActionValue& Value)
{
    Jump();
}

// ─── Combat ──────────────────────────────────────────────────────────────────

void ARVCharacterPlayer::InputAttack(const FInputActionValue& Value)
{
    if (IsValid(ComboComponent))
    {
        ComboComponent->HandleComboInput();
    }
}

void ARVCharacterPlayer::InputDodge(const FInputActionValue& Value)
{
    if (!IsValid(CombatComponent)) { return; }

    // Use last non-zero movement input for directional dodge
    // Falls back to character forward when standing still
    FVector DodgeDir = GetLastMovementInputVector();
    if (DodgeDir.IsNearlyZero())
    {
        DodgeDir = GetActorForwardVector();
    }

    CombatComponent->StartDodge(DodgeDir.GetSafeNormal());
}

void ARVCharacterPlayer::InputSprintStarted(const FInputActionValue& Value)
{
}

void ARVCharacterPlayer::InputSprintCompleted(const FInputActionValue& Value)
{
}

void ARVCharacterPlayer::InputGuardStarted(const FInputActionValue& Value)
{
    if (IsValid(CombatComponent))
    {
        CombatComponent->StartGuard();
    }
}

void ARVCharacterPlayer::InputGuardCompleted(const FInputActionValue& Value)
{
    if (IsValid(CombatComponent))
    {
        CombatComponent->EndGuard();
    }
}