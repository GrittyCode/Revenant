#include "Character/Player/RVCharacterPlayer.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Component/Combat/RVWeaponAttackComponent.h"
#include "Component/Combat/RVGuardComponent.h"
#include "Component/Combat/RVHitReactionComponent.h"
#include "Component/Utility/RVLockOnComponent.h"
#include "Data/Asset/RVPlayerDataAsset.h"
#include "Data/Asset/RVWeaponDataAsset.h"
#include "Data/Asset/RVHitReactionAnimDataAsset.h"
#include "Data/Row/RVWeaponStatRow.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Input/RVInputConfig.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Data/Asset/RVPlayerCombatAnimDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

static constexpr float MinSpeedToStartSprint = 10.f;

ARVCharacterPlayer::ARVCharacterPlayer()
{
    PrimaryActorTick.bCanEverTick = true;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength          = 450.f;
    CameraBoom->SocketOffset             = FVector(0.f, 0.f, 80.f);
    CameraBoom->bUsePawnControlRotation  = true;
    CameraBoom->bEnableCameraLag         = true;
    CameraBoom->CameraLagSpeed           = 7.f;
    CameraBoom->CameraLagMaxDistance     = 150.f;
    CameraBoom->bEnableCameraRotationLag = true;
    CameraBoom->CameraRotationLagSpeed   = 10.f;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
    FollowCamera->FieldOfView = 75.f;

    LockOnComponent       = CreateDefaultSubobject<URVLockOnComponent>      (TEXT("LockOnComponent"));
    WeaponAttackComponent = CreateDefaultSubobject<URVWeaponAttackComponent>(TEXT("WeaponAttackComponent"));
    GuardComponent        = CreateDefaultSubobject<URVGuardComponent>       (TEXT("GuardComponent"));
    StaminaComponent      = CreateDefaultSubobject<URVStaminaComponent>     (TEXT("StaminaComponent"));
    EquipmentComponent    = CreateDefaultSubobject<URVEquipmentComponent>   (TEXT("EquipmentComponent"));
}

void ARVCharacterPlayer::InitStats()
{
    if (!ensureMsgf(IsValid(PlayerData),
        TEXT("[%s] InitStats: PlayerData not assigned — assign DA_PlayerData in BP_RVCharacterPlayer"),
        *GetName())) { return; }

    const FRVPlayerStatRow* Stat = PlayerData->GetPlayerStatRow();
    if (!ensureMsgf(Stat,
        TEXT("[%s] InitStats: PlayerStatRowHandle resolve failed — check DT_PlayerStats"),
        *GetName())) { return; }

    VitalComponent->InitFromStatRow(*Stat);
    StaminaComponent->InitFromStatRow(*Stat);
    CachedDodgeStaminaCost = Stat->DodgeStaminaCost;

    HitReactionComponent->InitParams(
        GetHitReactionAnimData(),
        Stat->StaggerDuration,
        Stat->StaggerThreshold,
        Stat->KnockdownThreshold);

    GetCharacterMovement()->MaxWalkSpeed = Stat->MoveSpeed;
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

    PC->PlayerCameraManager->ViewPitchMin = -70.f;
    PC->PlayerCameraManager->ViewPitchMax =  20.f;

    // Inject sibling component references — components must not cast to the owning
    // character's concrete subtype to acquire these themselves.
    WeaponAttackComponent->Init(StaminaComponent, EquipmentComponent);
    GuardComponent->Init(StaminaComponent, EquipmentComponent);

    CombatStateComponent->OnForceEnd.AddUObject(WeaponAttackComponent, &URVWeaponAttackComponent::ForceEndAttack);
    CombatStateComponent->OnForceEnd.AddUObject(GuardComponent,        &URVGuardComponent::EndGuard);

    StaminaComponent->OnStaminaDepleted.AddDynamic(
        GuardComponent, &URVGuardComponent::OnStaminaDepletedHandler);

    CombatStateComponent->OnStateChanged.AddUObject(this, &ARVCharacterPlayer::OnCombatStateChangedForSprint);
    CombatStateComponent->OnForceEnd.AddUObject(this,     &ARVCharacterPlayer::EndSprint);

    OnHitConfirmed.AddUObject(this, &ARVCharacterPlayer::OnHitConfirmedHandler);

    EquipmentComponent->OnWeaponChanged.AddDynamic(this, &ARVCharacterPlayer::OnWeaponChangedHandler);
    OnWeaponChangedHandler(EquipmentComponent->GetCurrentWeaponData());
}

//--- Facades -----------------------------------------------------------------

FRVOnWeaponChanged&  ARVCharacterPlayer::GetOnWeaponChanged()       { return EquipmentComponent->OnWeaponChanged; }
FRVOnStaminaChanged& ARVCharacterPlayer::GetOnStaminaChanged()      { return StaminaComponent->OnStaminaChanged; }
float                ARVCharacterPlayer::GetStaminaRatio()    const { return StaminaComponent->GetStaminaPercent(); }

float ARVCharacterPlayer::GetSprintSpeed() const { return SprintSpeed; }
bool  ARVCharacterPlayer::IsSprinting()    const { return bIsSprinting; }
bool  ARVCharacterPlayer::IsComboActive()  const { return WeaponAttackComponent->IsLightAttackActive(); }
bool  ARVCharacterPlayer::IsLockedOn()     const { return LockOnComponent->IsLockedOn(); }

//--- AnimNotify forwarding ---------------------------------------------------

void ARVCharacterPlayer::OpenComboWindow()              { WeaponAttackComponent->OpenComboWindow(); }
void ARVCharacterPlayer::CloseComboWindow()             { WeaponAttackComponent->CloseComboWindow(); }
void ARVCharacterPlayer::TryChainCombo()                { WeaponAttackComponent->TryChainNextCombo(); }
void ARVCharacterPlayer::SetHeavyAttackReady(bool b)    { WeaponAttackComponent->SetHeavyAttackReady(b); }

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
        SetCombatStat(WeaponStat->BaseDamage, WeaponStat->BasePoiseDamage, WeaponStat->AttackRadius);
    }
}

//--- Hit FX ------------------------------------------------------------------

void ARVCharacterPlayer::OnHitConfirmedHandler(FVector ImpactLocation)
{
    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData)) { return; }

    if (IsValid(WeaponData->HitImpactEffect))
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), WeaponData->HitImpactEffect, ImpactLocation);
    }
    if (IsValid(WeaponData->HitSFX))
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponData->HitSFX, ImpactLocation);
    }
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

    if (HasCombatState(ERVCombatState::Attacking))
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

    ForceEndAllActions();

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
    if (HasCombatState(ERVCombatState::HitReaction))  { return; }
    if (WeaponAttackComponent->IsJumpAttackLanding()) { return; }

    const FVector2D Axis  = Value.Get<FVector2D>();
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

//--- Dodge -------------------------------------------------------------------

bool ARVCharacterPlayer::CanStartDodge() const
{
    if (HasCombatState(ERVCombatState::Dodging)) { return false; }
    if (!CanAct())                               { return false; }
    if (!IsGrounded())                           { return false; }
    return true;
}

void ARVCharacterPlayer::StartDodge(UAnimMontage* InMontage)
{
    if (!ensureMsgf(IsValid(InMontage),
        TEXT("[%s] StartDodge: Montage is null — check WeaponDataAsset dodge montage assignments"),
        *GetName())) { return; }

    if (!StaminaComponent->ConsumeStamina(CachedDodgeStaminaCost)) { return; }

    StaminaComponent->PauseStaminaRegen();
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
    StaminaComponent->ResumeStaminaRegen();

    // Restore orient-to-movement only when not locked on.
    GetCharacterMovement()->bOrientRotationToMovement = !IsLockedOn();
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
    if (StaminaComponent->GetCurrentStamina() <= 0.f)                           { return; }
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