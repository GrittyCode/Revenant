#include "Character/Player/RVCharacterPlayer.h"
#include "Camera/CameraComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVWeaponAttackComponent.h"
#include "Component/RVDodgeComponent.h"
#include "Component/RVGuardComponent.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVHitReactionComponent.h"
#include "Component/RVLockOnComponent.h"
#include "Data/RVPlayerDataAsset.h"
#include "Data/RVCharacterStatRow.h"
#include "Data/RVWeaponDataAsset.h"
#include "Data/RVPlayerCombatAnimDataAsset.h"
#include "Interface/RVDamageable.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Input/RVInputConfig.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Data/RVHitReactionAnimDataAsset.h"
#include "Data/RVWeaponStatRow.h"

// Must be moving at least this fast before sprint can activate.
static constexpr float MinSpeedToStartSprint = 10.f;

ARVCharacterPlayer::ARVCharacterPlayer()
{
    PrimaryActorTick.bCanEverTick = true;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 450.f;
    CameraBoom->SocketOffset = FVector(0.f, 0.f, 80.f);
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 1.f;
    CameraBoom->bEnableCameraRotationLag = false;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
    FollowCamera->FieldOfView = 75.f;

    LockOnComponent       = CreateDefaultSubobject<URVLockOnComponent>      (TEXT("LockOnComponent"));
    WeaponAttackComponent = CreateDefaultSubobject<URVWeaponAttackComponent>(TEXT("WeaponAttackComponent"));
    DodgeComponent        = CreateDefaultSubobject<URVDodgeComponent>       (TEXT("DodgeComponent"));
    GuardComponent        = CreateDefaultSubobject<URVGuardComponent>       (TEXT("GuardComponent"));
    EquipmentComponent    = CreateDefaultSubobject<URVEquipmentComponent>   (TEXT("EquipmentComponent"));
}

void ARVCharacterPlayer::InitStats()
{
    const URVPlayerDataAsset* PlayerData = Cast<URVPlayerDataAsset>(CharacterData);
    if (!IsValid(PlayerData)) { return; }

    const FRVCharacterStatRow* StatRow = PlayerData->GetStatRow();
    if (!StatRow) { return; }

    AttributeComponent->InitFromStatRow(*StatRow);
}

void ARVCharacterPlayer::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!ensureMsgf(IsValid(PC), TEXT("[%s] PlayerController missing"), *GetName())) { return; }

    UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
    if (!ensureMsgf(IsValid(Subsystem), TEXT("[%s] EnhancedInputLocalPlayerSubsystem missing"), *GetName())) { return; }
    if (!ensureMsgf(IsValid(DefaultMappingContext), TEXT("[%s] DefaultMappingContext not assigned"), *GetName())) { return; }

    Subsystem->AddMappingContext(DefaultMappingContext, 0);

    if (IsValid(PC->PlayerCameraManager))
    {
        PC->PlayerCameraManager->ViewPitchMin = -70.f;
        PC->PlayerCameraManager->ViewPitchMax =  20.f;
    }

    //--- Reference Injection -------------------------------------------------

    const URVPlayerDataAsset* PlayerData = Cast<URVPlayerDataAsset>(CharacterData);
    const float DodgeStaminaCost = IsValid(PlayerData) ? PlayerData->DodgeStaminaCost : 30.f;

    LockOnComponent->InitReferences(this, PC, CombatStateComponent);
    WeaponAttackComponent->InitReferences(this, CombatStateComponent, AttributeComponent, EquipmentComponent);
    DodgeComponent->InitReferences(this, CombatStateComponent, AttributeComponent, DodgeStaminaCost);
    GuardComponent->InitReferences(this, CombatStateComponent, AttributeComponent, EquipmentComponent);

    //--- Delegate Wiring -----------------------------------------------------

    AttributeComponent->OnStaminaDepleted.AddDynamic(
        GuardComponent, &URVGuardComponent::OnStaminaDepletedHandler);

    GuardComponent->OnGuardBreakTriggered.AddUObject(
        HitReactionComponent, &URVHitReactionComponent::TriggerStaggerWithMontage);

    WeaponAttackComponent->OnLightAttackStarted.AddUObject(
        CombatStateComponent, &URVCombatStateComponent::OnAttackStarted);
    WeaponAttackComponent->OnLightAttackEnded.AddUObject(
        CombatStateComponent, &URVCombatStateComponent::OnAttackEnded);

    CombatStateComponent->OnForceEnd.AddUObject(DodgeComponent, &URVDodgeComponent::ForceEndDodge);
    CombatStateComponent->OnForceEnd.AddUObject(GuardComponent,  &URVGuardComponent::EndGuard);

    // Sprint self-termination — ends automatically when any blocking combat state becomes active.
    CombatStateComponent->OnStateChanged.AddUObject(this, &ARVCharacterPlayer::OnCombatStateChangedForSprint);
    CombatStateComponent->OnForceEnd.AddUObject(this, &ARVCharacterPlayer::EndSprint);

    EquipmentComponent->OnWeaponChanged.AddDynamic(this, &ARVCharacterPlayer::OnWeaponChangedHandler);
    OnWeaponChangedHandler(EquipmentComponent->GetCurrentWeaponData());
}

//--- Component facades -------------------------------------------------------

FRVOnWeaponChanged& ARVCharacterPlayer::GetOnWeaponChanged()
{
    return EquipmentComponent->OnWeaponChanged;
}

URVWeaponDataAsset* ARVCharacterPlayer::GetCurrentWeaponData() const
{
    return EquipmentComponent->GetCurrentWeaponData();
}

bool ARVCharacterPlayer::IsComboActive() const
{
    return WeaponAttackComponent->IsComboActive();
}

float ARVCharacterPlayer::GetSprintSpeed() const
{
    return SprintSpeed;
}

bool ARVCharacterPlayer::IsSprinting() const
{
    return bIsSprinting;
}

bool ARVCharacterPlayer::IsLockedOn() const
{
    return LockOnComponent->IsLockedOn();
}

//--- GetHitReactionAnim / GetWeaponTraceMesh ---------------------------------

URVHitReactionAnimDataAsset* ARVCharacterPlayer::GetHitReactionAnimData() const
{
    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    return IsValid(WeaponData) ? WeaponData->HitReactionAnimData : nullptr;
}

UMeshComponent* ARVCharacterPlayer::GetWeaponTraceMesh() const
{
    return EquipmentComponent->GetWeaponMeshComponent();
}

void ARVCharacterPlayer::ActivateWeaponTrail()
{
    EquipmentComponent->ActivateWeaponTrail();
}

void ARVCharacterPlayer::DeactivateWeaponTrail()
{
    EquipmentComponent->DeactivateWeaponTrail();
}

void ARVCharacterPlayer::OnWeaponChangedHandler(URVWeaponDataAsset* NewWeaponData)
{
    URVHitReactionAnimDataAsset* NewCombatData = IsValid(NewWeaponData) ? NewWeaponData->HitReactionAnimData : nullptr;
    HitReactionComponent->SetHitReactionAnimData(NewCombatData);

    const FRVWeaponStatRow* WeaponStat = IsValid(NewWeaponData) ? NewWeaponData->GetWeaponStatRow() : nullptr;
    if (WeaponStat)
    {
        CombatStateComponent->SetCombatStat(
            WeaponStat->BaseDamage,
            WeaponStat->BasePoiseDamage,
            WeaponStat->AttackRadius);
    }

    CombatStateComponent->SetHitFX(
        IsValid(NewWeaponData) ? NewWeaponData->HitImpactEffect : nullptr,
        nullptr,
        IsValid(NewWeaponData) ? NewWeaponData->HitSFX          : nullptr);
}

//--- IRVDamageable -----------------------------------------------------------

bool ARVCharacterPlayer::ApplyDamage(const FRVHitInfo& InHitInfo)
{
    if (CombatStateComponent->IsInvincible()) { return false; }

    if (CombatStateComponent->HasState(ERVCombatState::Guarding))
    {
        GuardComponent->HandleGuardHit(InHitInfo.Damage);
        return true;
    }

    return Super::ApplyDamage(InHitInfo);
}

//--- Tick --------------------------------------------------------------------

void ARVCharacterPlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (LockOnComponent->IsLockedOn()) { return; }

    if (CombatStateComponent->HasState(ERVCombatState::Attacking | ERVCombatState::HeavyCharging | ERVCombatState::HeavyAttacking))
    {
        const FRotator CurrentRot = GetActorRotation();
        const FRotator TargetRot  = FRotator(0.f, AttackStartYaw, 0.f);
        SetActorRotation(FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, AttackRotationInterpSpeed));
    }
}

//--- Attack Direction --------------------------------------------------------

void ARVCharacterPlayer::SnapToAttackDirection()
{
    if (LockOnComponent->IsLockedOn())
    {
        AActor* Target = LockOnComponent->GetLockOnTarget();
        if (IsValid(Target))
        {
            FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
            ToTarget.Z = 0.f;
            const FRotator SnapRot = ToTarget.ToOrientationRotator();
            SetActorRotation(SnapRot);
            AttackStartYaw = SnapRot.Yaw;
        }
        return;
    }

    AttackStartYaw = GetActorRotation().Yaw;
}

void ARVCharacterPlayer::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);
    WeaponAttackComponent->OnPlayerLanded();
}

void ARVCharacterPlayer::OnDeath()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (IsValid(PC)) { DisableInput(PC); }

    URVHitReactionAnimDataAsset* HitReactionData = GetHitReactionAnimData();
    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();

    AnimInst->Montage_Stop(0.1f);
    AnimInst->Montage_Play(HitReactionData->DeathMontage);
}

//--- Input Setup -------------------------------------------------------------

void ARVCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* Eic = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!IsValid(Eic) || !IsValid(InputConfig)) { return; }

    Eic->BindAction(InputConfig->MoveAction, ETriggerEvent::Triggered, this, &ARVCharacterPlayer::InputMove);
    Eic->BindAction(InputConfig->LookAction, ETriggerEvent::Triggered, this, &ARVCharacterPlayer::InputLook);
    Eic->BindAction(InputConfig->JumpAction, ETriggerEvent::Started,   this, &ARVCharacterPlayer::InputJump);

    Eic->BindAction(InputConfig->AttackAction,        ETriggerEvent::Started,   this, &ARVCharacterPlayer::InputAttack);
    Eic->BindAction(InputConfig->HeavyAttackAction,   ETriggerEvent::Started,   this, &ARVCharacterPlayer::InputHeavyAttackStarted);
    Eic->BindAction(InputConfig->HeavyAttackAction,   ETriggerEvent::Completed, this, &ARVCharacterPlayer::InputHeavyAttackCompleted);
    Eic->BindAction(InputConfig->HeavyModifierAction, ETriggerEvent::Completed, this, &ARVCharacterPlayer::InputHeavyAttackCompleted);

    Eic->BindAction(InputConfig->DodgeAction, ETriggerEvent::Triggered, this, &ARVCharacterPlayer::InputDodge);

    Eic->BindAction(InputConfig->SprintAction, ETriggerEvent::Triggered, this, &ARVCharacterPlayer::InputSprintStarted);
    Eic->BindAction(InputConfig->SprintAction, ETriggerEvent::Completed, this, &ARVCharacterPlayer::InputSprintCompleted);

    Eic->BindAction(InputConfig->GuardAction, ETriggerEvent::Started,   this, &ARVCharacterPlayer::InputGuardStarted);
    Eic->BindAction(InputConfig->GuardAction, ETriggerEvent::Completed, this, &ARVCharacterPlayer::InputGuardCompleted);

    if (IsValid(InputConfig->LockOnAction))
    {
        Eic->BindAction(InputConfig->LockOnAction, ETriggerEvent::Started, this, &ARVCharacterPlayer::InputLockOn);
    }

    if (IsValid(InputConfig->WeaponSwapAction))
    {
        Eic->BindAction(InputConfig->WeaponSwapAction, ETriggerEvent::Started, this, &ARVCharacterPlayer::InputWeaponSwap);
    }
}

//--- Movement ----------------------------------------------------------------

void ARVCharacterPlayer::InputMove(const FInputActionValue& Value)
{
    if (CombatStateComponent->HasState(ERVCombatState::HitReaction)) { return; }
    if (WeaponAttackComponent->IsJumpAttackLanding()) { return; }

    const FVector2D Axis = Value.Get<FVector2D>();
    const FRotator YawOnly(0.f, GetControlRotation().Yaw, 0.f);

    AddMovementInput(FRotationMatrix(YawOnly).GetUnitAxis(EAxis::X), Axis.X);
    AddMovementInput(FRotationMatrix(YawOnly).GetUnitAxis(EAxis::Y), Axis.Y);
}

void ARVCharacterPlayer::InputLook(const FInputActionValue& Value)
{
    if (LockOnComponent->IsLockedOn()) { return; }

    const FVector2D Axis = Value.Get<FVector2D>();
    AddControllerYawInput  (Axis.X);
    AddControllerPitchInput(Axis.Y);
}

void ARVCharacterPlayer::InputJump(const FInputActionValue& Value)
{
    if (!CombatStateComponent->CheckAvailableState(ERVCombatState::Guarding)) { return; }
    Jump();
}

//--- Combat ------------------------------------------------------------------

void ARVCharacterPlayer::InputAttack(const FInputActionValue& Value)
{
    SnapToAttackDirection();
    WeaponAttackComponent->HandleLightAttackInput(bIsSprinting);
}

void ARVCharacterPlayer::InputHeavyAttackStarted(const FInputActionValue& Value)
{
    SnapToAttackDirection();
    WeaponAttackComponent->StartHeavyAttack();
}

void ARVCharacterPlayer::InputHeavyAttackCompleted(const FInputActionValue& Value)
{
    WeaponAttackComponent->ReleaseHeavyAttack();
}

void ARVCharacterPlayer::InputDodge(const FInputActionValue& Value)
{
    if (!DodgeComponent->CanStartDodge()) { return; }

    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData)) { return; }

    FVector DodgeDir = GetLastMovementInputVector();
    if (DodgeDir.IsNearlyZero()) { DodgeDir = GetActorForwardVector(); }
    DodgeDir = DodgeDir.GetSafeNormal();

    if (CombatStateComponent->HasState(ERVCombatState::Guarding)) { GuardComponent->EndGuard(); }

    UAnimMontage* Montage = nullptr;

    if (LockOnComponent->IsLockedOn())
    {
        const FVector Forward = GetActorForwardVector();
        const FVector Right   = GetActorRightVector();
        const float Angle = FMath::RadiansToDegrees(
            FMath::Atan2(FVector::DotProduct(Right, DodgeDir), FVector::DotProduct(Forward, DodgeDir)));

        if      (Angle > -67.5f  && Angle <=  67.5f)  { SetActorRotation(DodgeDir.ToOrientationRotator());   Montage = WeaponData->GetLockOnDodgeMontage(ERVDodgeDirection::Forward);   }
        else if (Angle >  67.5f  && Angle <= 112.5f)  {                                                       Montage = WeaponData->GetLockOnDodgeMontage(ERVDodgeDirection::Right);     }
        else if (Angle > 112.5f  && Angle <= 157.5f)  { SetActorRotation((-DodgeDir).ToOrientationRotator()); Montage = WeaponData->GetLockOnDodgeMontage(ERVDodgeDirection::BackRight);  }
        else if (Angle >  157.5f || Angle < -157.5f)  { SetActorRotation((-DodgeDir).ToOrientationRotator()); Montage = Angle > 0.f ? WeaponData->GetLockOnDodgeMontage(ERVDodgeDirection::BackRight) : WeaponData->GetLockOnDodgeMontage(ERVDodgeDirection::BackLeft); }
        else if (Angle < -112.5f && Angle >= -157.5f) { SetActorRotation((-DodgeDir).ToOrientationRotator()); Montage = WeaponData->GetLockOnDodgeMontage(ERVDodgeDirection::BackLeft);   }
        else                                           {                                                       Montage = WeaponData->GetLockOnDodgeMontage(ERVDodgeDirection::Left);      }
    }
    else
    {
        SetActorRotation(DodgeDir.ToOrientationRotator());
        Montage = WeaponData->GetDodgeMontage();
    }

    DodgeComponent->StartDodge(Montage);
}

void ARVCharacterPlayer::InputSprintStarted(const FInputActionValue& Value)
{
    StartSprint();
}

void ARVCharacterPlayer::InputSprintCompleted(const FInputActionValue& Value)
{
    EndSprint();
}

void ARVCharacterPlayer::InputGuardStarted(const FInputActionValue& Value)
{
    GuardComponent->StartGuard();
}

void ARVCharacterPlayer::InputGuardCompleted(const FInputActionValue& Value)
{
    GuardComponent->EndGuard();
}

void ARVCharacterPlayer::InputLockOn(const FInputActionValue& Value)
{
    if (bIsSprinting) { EndSprint(); }
    LockOnComponent->ToggleLockOn();
}

//--- Sprint ------------------------------------------------------------------

void ARVCharacterPlayer::StartSprint()
{
    if (bIsSprinting) { return; }
    if (LockOnComponent->IsLockedOn()) { return; }
    if (!CombatStateComponent->CheckAvailableState()) { return; }
    if (!CombatStateComponent->IsGrounded()) { return; }
    if (AttributeComponent->GetCurrentStamina() <= 0.f) { return; }

    // Require actual movement — prevents sprint activating from a standstill
    // when Space is held through an attack (InputSprintStarted fires every frame).
    if (GetCharacterMovement()->Velocity.Size2D() < MinSpeedToStartSprint) { return; }

    OriginalWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
    GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
    bIsSprinting = true;
}

void ARVCharacterPlayer::EndSprint()
{
    if (!bIsSprinting) { return; }
    bIsSprinting = false;
    GetCharacterMovement()->MaxWalkSpeed = OriginalWalkSpeed;
}

void ARVCharacterPlayer::OnCombatStateChangedForSprint(ERVCombatState InNewState)
{
    if (!bIsSprinting) { return; }
    if (!CombatStateComponent->CheckAvailableState()) { EndSprint(); }
}

//--- Weapon Swap -------------------------------------------------------------

void ARVCharacterPlayer::InputWeaponSwap(const FInputActionValue& Value)
{
    if (!CombatStateComponent->CheckAvailableState(ERVCombatState::Guarding)) { return; }
    EquipmentComponent->SwapWeapon();
}