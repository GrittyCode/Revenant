#include "Character/Player/RVCharacterPlayer.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "Component/RVComboComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Input/RVInputConfig.h"

ARVCharacterPlayer::ARVCharacterPlayer()
{
	// SpringArm handles camera collision so the camera never clips into walls.
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 400.0f;
	SpringArm->bUsePawnControlRotation = true; // Arm follows controller rotation

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; // Arm rotates; camera stays fixed relative to arm

	// Only the arm inherits controller yaw — prevents character mesh from rotating with the camera.
	bUseControllerRotationYaw   = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll  = false;

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
	if (!ensureMsgf(IsValid(EnhancedInput), TEXT("[ARVCharacterPlayer] EnhancedInputComponent not found")))
	{
		return;
	}

	if (!ensureMsgf(IsValid(InputConfig), TEXT("[ARVCharacterPlayer] InputConfig not assigned in Blueprint defaults")))
	{
		return;
	}

	EnhancedInput->BindAction(InputConfig->MoveAction,   ETriggerEvent::Triggered, this, &ARVCharacterPlayer::Move);
	EnhancedInput->BindAction(InputConfig->LookAction,   ETriggerEvent::Triggered, this, &ARVCharacterPlayer::Look);
	EnhancedInput->BindAction(InputConfig->JumpAction,   ETriggerEvent::Started,   this, &ACharacter::Jump);
	EnhancedInput->BindAction(InputConfig->JumpAction,   ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	EnhancedInput->BindAction(InputConfig->AttackAction, ETriggerEvent::Started,   this, &ARVCharacterPlayer::HandleAttackInput);
}

void ARVCharacterPlayer::Move(const FInputActionValue& InValue)
{
	// Unreal axis convention: X = forward/back, Y = left/right, Z = up/down
	const FVector2D MovementVector = InValue.Get<FVector2D>();

	if (!IsValid(Controller))
	{
		return;
	}

	// Derive movement directions from the controller yaw so the player moves relative to the camera.
	const FRotator Rotation    = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.X);
	AddMovementInput(RightDirection,   MovementVector.Y);
}

void ARVCharacterPlayer::Look(const FInputActionValue& InValue)
{
	const FVector2D LookVector = InValue.Get<FVector2D>();

	AddControllerYawInput(LookVector.X);
	AddControllerPitchInput(LookVector.Y);
}

void ARVCharacterPlayer::HandleAttackInput(const FInputActionValue& InValue)
{
	// ComboComponent is created via CreateDefaultSubobject in ARVCharacterBase::InitializeComponents
	// and is therefore guaranteed valid — no IsValid guard needed.
	ComboComponent->HandleComboInput();
}