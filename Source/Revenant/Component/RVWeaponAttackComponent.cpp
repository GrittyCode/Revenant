#include "Component/RVWeaponAttackComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Data/RVWeaponDataAsset.h"
#include "Data/RVPlayerCombatAnimDataAsset.h"
#include "Data/RVAttackActionMultiplierRow.h"
#include "Data/RVWeaponStatRow.h"
#include "Data/RVMontageStatData.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"

// Montage section names for the jump attack (Begin → Loop → Landing).
static const FName JumpAttackSection_Begin   = FName("Begin");
static const FName JumpAttackSection_Loop    = FName("Loop");
static const FName JumpAttackSection_Landing = FName("Landing");

URVWeaponAttackComponent::URVWeaponAttackComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URVWeaponAttackComponent::BeginPlay()
{
    Super::BeginPlay();
}

void URVWeaponAttackComponent::InitReferences(
    ACharacter* InOwnerCharacter,
    URVCombatStateComponent* InCombatStateComponent,
    URVAttributeComponent* InAttributeComponent,
    URVEquipmentComponent* InEquipmentComponent)
{
    OwnerCharacter       = InOwnerCharacter;
    CombatStateComponent = InCombatStateComponent;
    AttributeComponent   = InAttributeComponent;
    EquipmentComponent   = InEquipmentComponent;

    CombatStateComponent->OnForceEnd.AddUObject(this, &URVWeaponAttackComponent::ForceEndAttack);
}

//--- Light Attack ------------------------------------------------------------

void URVWeaponAttackComponent::HandleLightAttackInput(bool bIsPlayerSprinting)
{
    if (!bIsComboActive)
    {
        if (!CombatStateComponent->CheckAvailableState(ERVCombatState::Attacking)) { return; }

        if (!CombatStateComponent->IsGrounded())
        {
            if (!bHasUsedJumpAttack) { StartJumpAttack(); }
            return;
        }

        if (bIsPlayerSprinting)
        {
            StartRunAttack();
            return;
        }

        StartCombo();
        return;
    }

    if (bComboWindowOpen) { bHasComboInput = true; }
}

void URVWeaponAttackComponent::OpenComboWindow()
{
    bComboWindowOpen = true;
}

void URVWeaponAttackComponent::CloseComboWindow()
{
    bComboWindowOpen = false;
}

void URVWeaponAttackComponent::TryChainNextCombo()
{
    if (!bHasComboInput) { EndCombo(); return; }

    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData) || !IsValid(WeaponData->CombatAnimData)) { EndCombo(); return; }

    URVPlayerCombatAnimDataAsset* AnimData = WeaponData->CombatAnimData;
    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { EndCombo(); return; }

    const UAnimMontage* CurrentMontage = AnimInst->GetCurrentActiveMontage();
    const int32 CurrentIndex = AnimData->FindComboMontageIndex(CurrentMontage);

    if (CurrentIndex == INDEX_NONE) { EndCombo(); return; }

    UAnimMontage* NextMontage = AnimData->GetComboMontage(CurrentIndex + 1);
    if (!IsValid(NextMontage)) { EndCombo(); return; }

    bHasComboInput = false;

    if (!ConsumeAttackStamina(NextMontage, WeaponData)) { EndCombo(); return; }

    PlayLightAttackMontage(NextMontage);
}

void URVWeaponAttackComponent::OnPlayerLanded()
{
    // Always reset the per-jump gate regardless of attack state.
    bHasUsedJumpAttack = false;

    if (!bIsJumpAttackActive) { return; }

    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData) || !IsValid(WeaponData->CombatAnimData)) { return; }

    UAnimMontage* Montage = WeaponData->CombatAnimData->JumpAttackMontage;
    if (!IsValid(Montage)) { return; }

    // Block movement input and kill horizontal momentum for the landing animation.
    bIsJumpAttackLanding = true;
    UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
    MoveComp->Velocity.X = 0.f;
    MoveComp->Velocity.Y = 0.f;

    // Jump immediately to Landing — Montage_SetNextSection waits for the current
    // section cycle to finish, causing the character to float in the Loop pose.
    // Montage_JumpToSection transitions instantly on the next update tick.
    AnimInst->Montage_JumpToSection(JumpAttackSection_Landing, Montage);
}

void URVWeaponAttackComponent::StartCombo()
{
    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData) || !IsValid(WeaponData->CombatAnimData)) { return; }

    UAnimMontage* FirstMontage = WeaponData->CombatAnimData->GetComboMontage(0);
    if (!IsValid(FirstMontage)) { return; }

    if (!ConsumeAttackStamina(FirstMontage, WeaponData)) { return; }

    bIsComboActive = true;
    OnLightAttackStarted.Broadcast();

    PlayLightAttackMontage(FirstMontage);
}

void URVWeaponAttackComponent::StartRunAttack()
{
    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData) || !IsValid(WeaponData->CombatAnimData)) { return; }

    UAnimMontage* Montage = WeaponData->CombatAnimData->RunAttackMontage;
    if (!IsValid(Montage)) { return; }

    if (!ConsumeAttackStamina(Montage, WeaponData)) { return; }

    bIsComboActive = true;
    OnLightAttackStarted.Broadcast();

    PlayLightAttackMontage(Montage);
}

void URVWeaponAttackComponent::StartJumpAttack()
{
    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData) || !IsValid(WeaponData->CombatAnimData)) { return; }

    UAnimMontage* Montage = WeaponData->CombatAnimData->JumpAttackMontage;
    if (!IsValid(Montage)) { return; }

    if (!ConsumeAttackStamina(Montage, WeaponData)) { return; }

    bHasUsedJumpAttack   = true;
    bIsJumpAttackActive  = true;
    bIsComboActive       = true;
    OnLightAttackStarted.Broadcast();

    // Disable root motion before playing so the air attack animation
    // does not override the horizontal velocity from the jump.
    // Restored in EndCombo() when bIsJumpAttackActive is true.
    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (IsValid(AnimInst))
    {
        CachedRootMotionMode  = AnimInst->RootMotionMode;
        AnimInst->RootMotionMode = ERootMotionMode::IgnoreRootMotion;
    }

    PlayLightAttackMontage(Montage);
}

bool URVWeaponAttackComponent::ConsumeAttackStamina(
    UAnimMontage* InMontage, const URVWeaponDataAsset* InWeaponData)
{
    const URVMontageStatData* StatData = InMontage->GetAssetUserData<URVMontageStatData>();
    const FRVAttackActionMultiplierRow* AttackStat = StatData ? StatData->GetStatRow() : nullptr;
    if (!AttackStat || AttackStat->StaminaCostMultiplier <= 0.f) { return true; }

    const FRVWeaponStatRow* WeaponStat = InWeaponData->GetWeaponStatRow();
    const float Cost = WeaponStat
        ? WeaponStat->BaseStaminaCost * AttackStat->StaminaCostMultiplier
        : 0.f;

    return Cost <= 0.f || AttributeComponent->ConsumeStamina(Cost);
}

void URVWeaponAttackComponent::PlayLightAttackMontage(UAnimMontage* InMontage)
{
    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    AnimInst->Montage_Play(InMontage);

    FOnMontageBlendingOutStarted BlendingOutDelegate;
    BlendingOutDelegate.BindUObject(this, &URVWeaponAttackComponent::OnLightAttackMontageBlendingOut);
    AnimInst->Montage_SetBlendingOutDelegate(BlendingOutDelegate, InMontage);
}

void URVWeaponAttackComponent::EndCombo()
{
    // Restore root motion mode if we overrode it for a jump attack.
    if (bIsJumpAttackActive)
    {
        UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
        if (IsValid(AnimInst)) { AnimInst->RootMotionMode = CachedRootMotionMode; }
    }

    bIsComboActive       = false;
    bComboWindowOpen     = false;
    bHasComboInput       = false;
    bIsJumpAttackActive  = false;
    bIsJumpAttackLanding = false;

    OnLightAttackEnded.Broadcast();
}

void URVWeaponAttackComponent::OnLightAttackMontageBlendingOut(UAnimMontage* /*Montage*/, bool bInterrupted)
{
    if (!bInterrupted && bIsComboActive)
    {
        EndCombo();
    }
}

//--- Heavy Attack ------------------------------------------------------------

void URVWeaponAttackComponent::StartHeavyAttack()
{
    if (!CombatStateComponent->CheckAvailableState()) { return; }
    if (!CombatStateComponent->IsGrounded()) { return; }
    if (AttributeComponent->GetCurrentStamina() <= 0.f) { return; }

    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData)) { return; }

    UAnimMontage* ChargeMontage = WeaponData->GetHeavyChargeMontage();
    if (!IsValid(ChargeMontage)) { return; }

    CombatStateComponent->AddState(ERVCombatState::HeavyCharging);
    bCanHeavyRelease = false;
    bPendingRelease  = false;
    bIsAutoRelease   = false;

    // Reset regen delay without consuming stamina —
    // prevents regen from ticking silently during the charge window.
    AttributeComponent->ResetStaminaRegenDelay();

    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    FOnMontageBlendingOutStarted ChargeBlendOut;
    ChargeBlendOut.BindUObject(this, &URVWeaponAttackComponent::OnChargeMontageBlendingOut);

    AnimInst->Montage_Play(ChargeMontage);
    AnimInst->Montage_SetBlendingOutDelegate(ChargeBlendOut, ChargeMontage);

    GetWorld()->GetTimerManager().SetTimer(
        ChargeAutoReleaseHandle,
        this,
        &URVWeaponAttackComponent::OnChargeAutoRelease,
        MaxChargeTime,
        false);
}

void URVWeaponAttackComponent::ReleaseHeavyAttack()
{
    if (!CombatStateComponent->HasState(ERVCombatState::HeavyCharging)) { return; }

    if (!bCanHeavyRelease)
    {
        bPendingRelease = true;
        return;
    }

    ExecuteHeavyAttack();
}

void URVWeaponAttackComponent::SetHeavyAttackReady(bool bReady)
{
    bCanHeavyRelease = bReady;

    if (bReady && bPendingRelease)
    {
        bPendingRelease = false;
        ExecuteHeavyAttack();
    }
}

void URVWeaponAttackComponent::ExecuteHeavyAttack()
{
    GetWorld()->GetTimerManager().ClearTimer(ChargeAutoReleaseHandle);

    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData)) { EndHeavyAttack(); return; }

    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { EndHeavyAttack(); return; }

    UAnimMontage* ReleaseMontage = WeaponData->GetHeavyAttackMontage(bIsAutoRelease);
    if (!IsValid(ReleaseMontage)) { EndHeavyAttack(); return; }

    // Stamina consumed at release — also resets the regen delay clock via ConsumeStamina.
    const URVMontageStatData* StatData = ReleaseMontage->GetAssetUserData<URVMontageStatData>();
    const FRVAttackActionMultiplierRow* AttackStat = StatData ? StatData->GetStatRow() : nullptr;
    if (AttackStat && AttackStat->StaminaCostMultiplier > 0.f)
    {
        const FRVWeaponStatRow* WeaponStat = WeaponData->GetWeaponStatRow();
        if (WeaponStat)
        {
            AttributeComponent->ConsumeStamina(
                WeaponStat->BaseStaminaCost * AttackStat->StaminaCostMultiplier);
        }
    }

    bIsAutoRelease = false;

    // HeavyCharging removed before HeavyAttacking —
    // OnChargeMontageBlendingOut checks HeavyCharging to detect external interruption.
    CombatStateComponent->RemoveState(ERVCombatState::HeavyCharging);
    CombatStateComponent->AddState(ERVCombatState::HeavyAttacking);

    FOnMontageBlendingOutStarted ReleaseBlendOut;
    ReleaseBlendOut.BindUObject(this, &URVWeaponAttackComponent::OnReleaseMontageBlendingOut);

    AnimInst->Montage_Play(ReleaseMontage);
    AnimInst->Montage_SetBlendingOutDelegate(ReleaseBlendOut, ReleaseMontage);
}

void URVWeaponAttackComponent::EndHeavyAttack()
{
    if (!CombatStateComponent->HasState(ERVCombatState::HeavyCharging | ERVCombatState::HeavyAttacking))
    {
        return;
    }

    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (IsValid(AnimInst))
    {
        const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
        if (IsValid(WeaponData))
        {
            UAnimMontage* MontageToStop = CombatStateComponent->HasState(ERVCombatState::HeavyCharging)
                ? WeaponData->GetHeavyChargeMontage()
                : AnimInst->GetCurrentActiveMontage();

            if (IsValid(MontageToStop))
            {
                AnimInst->Montage_Stop(0.1f, MontageToStop);
            }
        }
    }

    CombatStateComponent->RemoveState(ERVCombatState::HeavyCharging | ERVCombatState::HeavyAttacking);
    bCanHeavyRelease = false;
    bPendingRelease  = false;
    bIsAutoRelease   = false;
    GetWorld()->GetTimerManager().ClearTimer(ChargeAutoReleaseHandle);
}

void URVWeaponAttackComponent::OnChargeAutoRelease()
{
    bIsAutoRelease = true;
    ReleaseHeavyAttack();
}

void URVWeaponAttackComponent::OnChargeMontageBlendingOut(UAnimMontage* /*InMontage*/, bool /*bInterrupted*/)
{
    if (CombatStateComponent->HasState(ERVCombatState::HeavyCharging))
    {
        EndHeavyAttack();
    }
}

void URVWeaponAttackComponent::OnReleaseMontageBlendingOut(UAnimMontage* /*InMontage*/, bool /*bInterrupted*/)
{
    EndHeavyAttack();
}

//--- Shared ------------------------------------------------------------------

void URVWeaponAttackComponent::ForceEndAttack()
{
    if (bIsComboActive)
    {
        UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
        if (IsValid(AnimInst))
        {
            UAnimMontage* Current = AnimInst->GetCurrentActiveMontage();
            if (IsValid(Current)) { AnimInst->Montage_Stop(0.1f, Current); }
        }
        EndCombo();
    }

    if (CombatStateComponent->HasState(ERVCombatState::HeavyCharging | ERVCombatState::HeavyAttacking))
    {
        EndHeavyAttack();
    }
}