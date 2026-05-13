#include "Character/Player/RVCharacterPlayer.h"
#include "Camera/CameraComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVHeavyAttackComponent.h"
#include "Component/RVDodgeComponent.h"
#include "Component/RVGuardComponent.h"
#include "Component/RVComboComponent.h"
#include "Component/RVSprintComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Data/RVWeaponDataAsset.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Input/RVInputConfig.h"
#include "GameFramework/SpringArmComponent.h"

ARVCharacterPlayer::ARVCharacterPlayer()
{
    PrimaryActorTick.bCanEverTick = true;

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

void ARVCharacterPlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Rotate toward camera yaw while attacking.
    // Skipped outside attack states to avoid interfering with Orient-to-Movement.
    if (CombatStateComponent->IsInState(ERVCombatState::Attacking | ERVCombatState::HeavyCharging | ERVCombatState::HeavyAttacking))
    {
        const FRotator ControlRot = GetControlRotation();
        const FRotator CurrentRot = GetActorRotation();
        const FRotator TargetRot  = FRotator(0.f, ControlRot.Yaw, 0.f);

        SetActorRotation(FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, AttackRotationInterpSpeed));
    }
}

void ARVCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* Eic = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!IsValid(Eic) || !IsValid(InputConfig)) { return; }

    // ─── Movement ────────────────────────────────────────────────────────────

    Eic->BindAction(InputConfig->MoveAction, ETriggerEvent::Triggered, this, &ARVCharacterPlayer::InputMove);
    Eic->BindAction(InputConfig->LookAction, ETriggerEvent::Triggered, this, &ARVCharacterPlayer::InputLook);
    Eic->BindAction(InputConfig->JumpAction, ETriggerEvent::Started,   this, &ARVCharacterPlayer::InputJump);

    // ─── Combat ──────────────────────────────────────────────────────────────

    Eic->BindAction(InputConfig->AttackAction,        ETriggerEvent::Started,   this, &ARVCharacterPlayer::InputAttack);
    Eic->BindAction(InputConfig->HeavyAttackAction,   ETriggerEvent::Started,   this, &ARVCharacterPlayer::InputHeavyAttackStarted);
    Eic->BindAction(InputConfig->HeavyAttackAction,   ETriggerEvent::Completed, this, &ARVCharacterPlayer::InputHeavyAttackCompleted);
    Eic->BindAction(InputConfig->HeavyModifierAction, ETriggerEvent::Completed, this, &ARVCharacterPlayer::InputHeavyAttackCompleted);

    // Dodge: Triggered fires after Tap threshold — Started fires before tap is confirmed.
    Eic->BindAction(InputConfig->DodgeAction, ETriggerEvent::Triggered, this, &ARVCharacterPlayer::InputDodge);

    // Sprint: Triggered fires when Hold threshold is met, Completed fires on release.
    Eic->BindAction(InputConfig->SprintAction, ETriggerEvent::Triggered, this, &ARVCharacterPlayer::InputSprintStarted);
    Eic->BindAction(InputConfig->SprintAction, ETriggerEvent::Completed, this, &ARVCharacterPlayer::InputSprintCompleted);

    // Guard: Started = RMB pressed, Completed = RMB released.
    Eic->BindAction(InputConfig->GuardAction, ETriggerEvent::Started,   this, &ARVCharacterPlayer::InputGuardStarted);
    Eic->BindAction(InputConfig->GuardAction, ETriggerEvent::Completed, this, &ARVCharacterPlayer::InputGuardCompleted);

    // Weapon swap: Tab — Phase 2 temp, replaced by ARVWeaponPickup in Phase 4.
    if (IsValid(InputConfig->WeaponSwapAction))
    {
        Eic->BindAction(InputConfig->WeaponSwapAction, ETriggerEvent::Started, this, &ARVCharacterPlayer::InputWeaponSwap);
    }
}

// ─── Movement ────────────────────────────────────────────────────────────────

void ARVCharacterPlayer::InputMove(const FInputActionValue& Value)
{
	if (CombatStateComponent->IsInState(ERVCombatState::HitReaction)) { return; }

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
    // Guard is coexistable — player cannot jump while in other blocking states.
    if (!CombatStateComponent->CheckAvailableState(ERVCombatState::Guarding)) { return; }
    Jump();
}

// ─── Combat ──────────────────────────────────────────────────────────────────

void ARVCharacterPlayer::InputAttack(const FInputActionValue& Value)
{
    ComboComponent->HandleComboInput();
}

void ARVCharacterPlayer::InputHeavyAttackStarted(const FInputActionValue& Value)
{
    HeavyAttackComponent->StartHeavyAttack();
}

void ARVCharacterPlayer::InputHeavyAttackCompleted(const FInputActionValue& Value)
{
    HeavyAttackComponent->ReleaseHeavyAttack();
}

void ARVCharacterPlayer::InputDodge(const FInputActionValue& Value)
{
    FVector DodgeDir = GetLastMovementInputVector();
    if (DodgeDir.IsNearlyZero())
    {
        DodgeDir = GetActorForwardVector();
    }

    DodgeComponent->StartDodge(DodgeDir.GetSafeNormal());
}

void ARVCharacterPlayer::InputSprintStarted(const FInputActionValue& Value)
{
    SprintComponent->StartSprint();
}

void ARVCharacterPlayer::InputSprintCompleted(const FInputActionValue& Value)
{
    SprintComponent->EndSprint();
}

void ARVCharacterPlayer::InputGuardStarted(const FInputActionValue& Value)
{
    GuardComponent->StartGuard();
}

void ARVCharacterPlayer::InputGuardCompleted(const FInputActionValue& Value)
{
    GuardComponent->EndGuard();
}

// ─── Weapon Swap (temp) ──────────────────────────────────────────────

void ARVCharacterPlayer::InputWeaponSwap(const FInputActionValue& Value)
{
    if (!CombatStateComponent->CheckAvailableState(ERVCombatState::Guarding)) { return; }

    bIsWeaponA = !bIsWeaponA;
    URVWeaponDataAsset* NextWeapon = bIsWeaponA ? WeaponDataA : WeaponDataB;

    if (!IsValid(NextWeapon)) { return; }

    EquipmentComponent->SetCurrentWeaponData(NextWeapon);
}