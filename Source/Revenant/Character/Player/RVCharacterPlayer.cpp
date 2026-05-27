#include "Character/Player/RVCharacterPlayer.h"
#include "Camera/CameraComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVWeaponAttackComponent.h"
#include "Component/RVGuardComponent.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVHitReactionComponent.h"
#include "Component/RVLockOnComponent.h"
#include "Data/RVPlayerDataAsset.h"
#include "Data/RVCharacterStatRow.h"
#include "Data/RVWeaponDataAsset.h"
#include "Data/RVPlayerCombatAnimDataAsset.h"
#include "Data/RVHitReactionAnimDataAsset.h"
#include "Data/RVWeaponStatRow.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Input/RVInputConfig.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Animation/AnimInstance.h"

static constexpr float MinSpeedToStartSprint = 10.f;

ARVCharacterPlayer::ARVCharacterPlayer()
{
    PrimaryActorTick.bCanEverTick = true;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength          = 450.f;
    CameraBoom->SocketOffset             = FVector(0.f, 0.f, 80.f);
    CameraBoom->bUsePawnControlRotation  = true;

    // Position lag: moderate catch-up speed gives a natural soulsy follow feel.
    // CameraLagMaxDistance caps rubber-banding on sudden large displacements (e.g. knockback).
    CameraBoom->bEnableCameraLag     = true;
    CameraBoom->CameraLagSpeed       = 7.f;
    CameraBoom->CameraLagMaxDistance = 150.f;

    // Rotation lag: adds a subtle delay when the player pans the camera,
    // preventing jarring snaps and improving cinematic polish.
    CameraBoom->bEnableCameraRotationLag = true;
    CameraBoom->CameraRotationLagSpeed   = 10.f;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
    FollowCamera->FieldOfView = 75.f;

    LockOnComponent       = CreateDefaultSubobject<URVLockOnComponent>      (TEXT("LockOnComponent"));
    WeaponAttackComponent = CreateDefaultSubobject<URVWeaponAttackComponent>(TEXT("WeaponAttackComponent"));
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
    DodgeStaminaCost = PlayerData->DodgeStaminaCost;
}

void ARVCharacterPlayer::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!ensureMsgf(IsValid(PC), TEXT("[%s] PlayerController missing"), *GetName())) { return; }

    UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
    if (!ensureMsgf(IsValid(Subsystem),
        TEXT("[%s] EnhancedInputLocalPlayerSubsystem missing"), *GetName())) { return; }
    if (!ensureMsgf(IsValid(DefaultMappingContext),
        TEXT("[%s] DefaultMappingContext not assigned"), *GetName())) { return; }

    Subsystem->AddMappingContext(DefaultMappingContext, 0);

    if (IsValid(PC->PlayerCameraManager))
    {
        PC->PlayerCameraManager->ViewPitchMin = -70.f;
        PC->PlayerCameraManager->ViewPitchMax =  20.f;
    }

    //--- Sibling delegate wiring (Player owns both sides of each connection) -
    // Components self-initialize their Owner reference in their own BeginPlay.
    // All cross-component delegate connections are the Player's responsibility.

    // ForceEnd cascades to all action components.
    CombatStateComponent->OnForceEnd.AddUObject(WeaponAttackComponent, &URVWeaponAttackComponent::ForceEndAttack);
    CombatStateComponent->OnForceEnd.AddUObject(GuardComponent,        &URVGuardComponent::EndGuard);

    // Stamina depletion breaks guard.
    AttributeComponent->OnStaminaDepleted.AddDynamic(
        GuardComponent, &URVGuardComponent::OnStaminaDepletedHandler);

    CombatStateComponent->OnStateChanged.AddUObject(this, &ARVCharacterPlayer::OnCombatStateChangedForSprint);
    CombatStateComponent->OnForceEnd.AddUObject(this,     &ARVCharacterPlayer::EndSprint);

    EquipmentComponent->OnWeaponChanged.AddDynamic(this, &ARVCharacterPlayer::OnWeaponChangedHandler);
    OnWeaponChangedHandler(EquipmentComponent->GetCurrentWeaponData());
}

//--- IRVWeaponUser -----------------------------------------------------------

URVWeaponDataAsset* ARVCharacterPlayer::GetCurrentWeaponData() const
{
    return EquipmentComponent->GetCurrentWeaponData();
}

//--- Facades -----------------------------------------------------------------

FRVOnWeaponChanged& ARVCharacterPlayer::GetOnWeaponChanged(){ return EquipmentComponent->OnWeaponChanged;}

bool ARVCharacterPlayer::IsComboActive()  const { return WeaponAttackComponent->IsComboActive(); }
float ARVCharacterPlayer::GetSprintSpeed() const { return SprintSpeed; }
bool  ARVCharacterPlayer::IsSprinting()    const { return bIsSprinting; }
bool  ARVCharacterPlayer::IsLockedOn()     const { return LockOnComponent->IsLockedOn(); }

//--- AnimNotify forwarding ---------------------------------------------------

void ARVCharacterPlayer::OpenComboWindow()           { WeaponAttackComponent->OpenComboWindow(); }
void ARVCharacterPlayer::CloseComboWindow()          { WeaponAttackComponent->CloseComboWindow(); }
void ARVCharacterPlayer::TryChainNextCombo()         { WeaponAttackComponent->TryChainNextCombo(); }
void ARVCharacterPlayer::SetHeavyAttackReady(bool bReady) { WeaponAttackComponent->SetHeavyAttackReady(bReady); }

//--- Overrides ---------------------------------------------------------------

URVHitReactionAnimDataAsset* ARVCharacterPlayer::GetHitReactionAnimData() const
{
    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    return IsValid(WeaponData) ? WeaponData->HitReactionAnimData : nullptr;
}

UMeshComponent* ARVCharacterPlayer::GetWeaponTraceMesh() const
{
    return EquipmentComponent->GetWeaponMeshComponent();
}

void ARVCharacterPlayer::ActivateWeaponTrail()   { EquipmentComponent->ActivateWeaponTrail(); }
void ARVCharacterPlayer::DeactivateWeaponTrail() { EquipmentComponent->DeactivateWeaponTrail(); }

void ARVCharacterPlayer::OnWeaponChangedHandler(URVWeaponDataAsset* NewWeaponData)
{
    HitReactionComponent->SetHitReactionAnimData(
        IsValid(NewWeaponData) ? NewWeaponData->HitReactionAnimData : nullptr);

    const FRVWeaponStatRow* WeaponStat =
        IsValid(NewWeaponData) ? NewWeaponData->GetWeaponStatRow() : nullptr;

    if (WeaponStat)
    {
        // Route through CharacterBase facade — no direct component access from Player.
        SetCombatStat(WeaponStat->BaseDamage, WeaponStat->BasePoiseDamage, WeaponStat->AttackRadius);
    }

    SetHitFX(
        IsValid(NewWeaponData) ? NewWeaponData->HitImpactEffect : nullptr,
        nullptr,
        IsValid(NewWeaponData) ? NewWeaponData->HitSFX          : nullptr);
}

//--- IRVDamageable -----------------------------------------------------------

bool ARVCharacterPlayer::ApplyDamage(const FRVHitInfo& InHitInfo)
{
    if (IsInvincible()) { return false; }

    if (HasCombatState(ERVCombatState::Guarding))
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

    if (HasCombatState(ERVCombatState::Attacking | ERVCombatState::HeavyCharging | ERVCombatState::HeavyAttacking))
    {
        SetActorRotation(FMath::RInterpTo(
            GetActorRotation(), FRotator(0.f, AttackStartYaw, 0.f), DeltaTime, AttackRotationInterpSpeed));
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

//--- Lifecycle ---------------------------------------------------------------

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
    if (!ensureMsgf(IsValid(HitReactionData),
        TEXT("[%s] OnDeath: HitReactionAnimData not assigned"), *GetName())) { return; }

    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    if (!ensureMsgf(IsValid(AnimInst),
        TEXT("[%s] OnDeath: AnimInstance missing"), *GetName())) { return; }

    AnimInst->Montage_Stop(0.1f);
    AnimInst->Montage_Play(HitReactionData->DeathMontage);
}

//--- Input Setup -------------------------------------------------------------

void ARVCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* Eic = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!ensureMsgf(IsValid(Eic) && IsValid(InputConfig),
        TEXT("[%s] EnhancedInputComponent or InputConfig missing"), *GetName())) { return; }

    Eic->BindAction(InputConfig->MoveAction, ETriggerEvent::Triggered, this, &ARVCharacterPlayer::InputMove);
    Eic->BindAction(InputConfig->LookAction, ETriggerEvent::Triggered, this, &ARVCharacterPlayer::InputLook);
    Eic->BindAction(InputConfig->JumpAction, ETriggerEvent::Started,   this, &ARVCharacterPlayer::InputJump);

    Eic->BindAction(InputConfig->AttackAction,        ETriggerEvent::Started,   this, &ARVCharacterPlayer::InputAttack);
    Eic->BindAction(InputConfig->HeavyAttackAction,   ETriggerEvent::Started,   this, &ARVCharacterPlayer::InputHeavyAttackStarted);
    Eic->BindAction(InputConfig->HeavyAttackAction,   ETriggerEvent::Completed, this, &ARVCharacterPlayer::InputHeavyAttackCompleted);
    Eic->BindAction(InputConfig->HeavyModifierAction, ETriggerEvent::Completed, this, &ARVCharacterPlayer::InputHeavyAttackCompleted);

    Eic->BindAction(InputConfig->DodgeAction,  ETriggerEvent::Triggered, this, &ARVCharacterPlayer::InputDodge);
    Eic->BindAction(InputConfig->SprintAction, ETriggerEvent::Triggered, this, &ARVCharacterPlayer::InputSprintStarted);
    Eic->BindAction(InputConfig->SprintAction, ETriggerEvent::Completed, this, &ARVCharacterPlayer::InputSprintCompleted);
    Eic->BindAction(InputConfig->GuardAction,  ETriggerEvent::Started,   this, &ARVCharacterPlayer::InputGuardStarted);
    Eic->BindAction(InputConfig->GuardAction,  ETriggerEvent::Completed, this, &ARVCharacterPlayer::InputGuardCompleted);

    if (IsValid(InputConfig->LockOnAction))
    {
        Eic->BindAction(InputConfig->LockOnAction, ETriggerEvent::Started, this, &ARVCharacterPlayer::InputLockOn);
    }
    if (IsValid(InputConfig->WeaponSwapAction))
    {
        Eic->BindAction(InputConfig->WeaponSwapAction, ETriggerEvent::Started, this, &ARVCharacterPlayer::InputWeaponSwap);
    }
}

//--- Input Handlers ----------------------------------------------------------

void ARVCharacterPlayer::InputMove(const FInputActionValue& Value)
{
    if (HasCombatState(ERVCombatState::HitReaction))      { return; }
    if (WeaponAttackComponent->IsJumpAttackLanding()) { return; }

    const FVector2D Axis   = Value.Get<FVector2D>();
    const FRotator  YawOnly(0.f, GetControlRotation().Yaw, 0.f);

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
    if (!CanAct(ERVCombatState::Guarding)) { return; }
    Jump();
}

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

//--- Dodge (inlined from deleted URVDodgeComponent) --------------------------

bool ARVCharacterPlayer::CanStartDodge() const
{
    if (HasCombatState(ERVCombatState::Dodging)) { return false; }
    if (!CanAct())                               { return false; }
    if (!IsGrounded())                           { return false; }
    // Stamina gating delegated to TryConsumeStamina inside StartDodge.
    return true;
}

void ARVCharacterPlayer::StartDodge(UAnimMontage* InMontage)
{
    if (!ensureMsgf(IsValid(InMontage),
        TEXT("[%s] StartDodge: Montage is null — check WeaponDataAsset dodge montage assignments"),
        *GetName())) { return; }

    if (!TryConsumeStamina(DodgeStaminaCost)) { return; }

    PauseStaminaRegen();
    SetInvincible(true);
    AddCombatState(ERVCombatState::Dodging);

    GetCharacterMovement()->bOrientRotationToMovement = !IsLockedOn();

    ActiveDodgeMontage = InMontage;

    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    ensureMsgf(IsValid(AnimInst), TEXT("[%s] StartDodge: AnimInstance missing"), *GetName());
    if (!IsValid(AnimInst)) { return; }

    AnimInst->Montage_Play(InMontage);

    FOnMontageBlendingOutStarted BlendOutDelegate;
    BlendOutDelegate.BindUObject(this, &ARVCharacterPlayer::OnDodgeMontageBlendingOut);
    AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, InMontage);
}

void ARVCharacterPlayer::EndDodge()
{
    if (!HasCombatState(ERVCombatState::Dodging)) { return; }

    SetInvincible(false);
    RemoveCombatState(ERVCombatState::Dodging);
    ResumeStaminaRegen();

    // Restore orientation — LockOnComponent re-suppresses next tick if still locked on.
    GetCharacterMovement()->bOrientRotationToMovement = true;
    ActiveDodgeMontage = nullptr;
}

void ARVCharacterPlayer::OnDodgeMontageBlendingOut(UAnimMontage*, bool)
{
    EndDodge();
}

void ARVCharacterPlayer::InputDodge(const FInputActionValue& Value)
{
    if (!CanStartDodge()) { return; }

    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData)) { return; }

    FVector DodgeDir = GetLastMovementInputVector();
    if (DodgeDir.IsNearlyZero()) { DodgeDir = GetActorForwardVector(); }
    DodgeDir = DodgeDir.GetSafeNormal();

    if (HasCombatState(ERVCombatState::Guarding)) { GuardComponent->EndGuard(); }

    UAnimMontage* Montage = nullptr;

    if (LockOnComponent->IsLockedOn())
    {
        const FVector Forward = GetActorForwardVector();
        const FVector Right   = GetActorRightVector();
        const float   Angle   = FMath::RadiansToDegrees(
            FMath::Atan2(FVector::DotProduct(Right, DodgeDir), FVector::DotProduct(Forward, DodgeDir)));

        if      (Angle > -67.5f  && Angle <=  67.5f)  { SetActorRotation(DodgeDir.ToOrientationRotator());    Montage = WeaponData->GetLockOnDodgeMontage(ERVDodgeDirection::Forward);  }
        else if (Angle >  67.5f  && Angle <= 112.5f)  {                                                        Montage = WeaponData->GetLockOnDodgeMontage(ERVDodgeDirection::Right);    }
        else if (Angle > 112.5f  && Angle <= 157.5f)  { SetActorRotation((-DodgeDir).ToOrientationRotator()); Montage = WeaponData->GetLockOnDodgeMontage(ERVDodgeDirection::BackRight); }
        else if (Angle >  157.5f || Angle < -157.5f)  { SetActorRotation((-DodgeDir).ToOrientationRotator()); Montage = Angle > 0.f ? WeaponData->GetLockOnDodgeMontage(ERVDodgeDirection::BackRight) : WeaponData->GetLockOnDodgeMontage(ERVDodgeDirection::BackLeft); }
        else if (Angle < -112.5f && Angle >= -157.5f) { SetActorRotation((-DodgeDir).ToOrientationRotator()); Montage = WeaponData->GetLockOnDodgeMontage(ERVDodgeDirection::BackLeft);  }
        else                                           {                                                        Montage = WeaponData->GetLockOnDodgeMontage(ERVDodgeDirection::Left);     }
    }
    else
    {
        SetActorRotation(DodgeDir.ToOrientationRotator());
        Montage = WeaponData->GetDodgeMontage();
    }

    StartDodge(Montage);
}

void ARVCharacterPlayer::InputSprintStarted  (const FInputActionValue&) { StartSprint(); }
void ARVCharacterPlayer::InputSprintCompleted(const FInputActionValue&) { EndSprint(); }
void ARVCharacterPlayer::InputGuardStarted   (const FInputActionValue&) { GuardComponent->StartGuard(); }
void ARVCharacterPlayer::InputGuardCompleted (const FInputActionValue&) { GuardComponent->EndGuard(); }

void ARVCharacterPlayer::InputLockOn(const FInputActionValue&)
{
    if (bIsSprinting) { EndSprint(); }
    LockOnComponent->ToggleLockOn();
}

//--- Sprint ------------------------------------------------------------------

void ARVCharacterPlayer::StartSprint()
{
    if (bIsSprinting)                  { return; }
    if (LockOnComponent->IsLockedOn()) { return; }
    if (!CanAct())                     { return; }
    if (!IsGrounded())                 { return; }
    if (GetCurrentStamina() <= 0.f)    { return; }
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

void ARVCharacterPlayer::OnCombatStateChangedForSprint(ERVCombatState)
{
    if (!bIsSprinting) { return; }
    if (!CanAct()) { EndSprint(); }
}

//--- Weapon Swap -------------------------------------------------------------

void ARVCharacterPlayer::InputWeaponSwap(const FInputActionValue&)
{
    if (!CanAct(ERVCombatState::Guarding)) { return; }
    EquipmentComponent->SwapWeapon();
}