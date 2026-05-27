#include "Component/RVWeaponAttackComponent.h"
#include "Character/Base/RVCharacterBase.h"
#include "Interface/RVWeaponUser.h"
#include "Data/RVWeaponDataAsset.h"
#include "Data/RVPlayerCombatAnimDataAsset.h"
#include "Data/RVAttackActionMultiplierRow.h"
#include "Data/RVWeaponStatRow.h"
#include "Data/RVMontageStatData.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"

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

    OwnerBase  = Cast<ARVCharacterBase>(GetOwner());
    WeaponUser = Cast<IRVWeaponUser>(GetOwner());

    ensureMsgf(IsValid(OwnerBase),
        TEXT("[URVWeaponAttackComponent] Owner must be ARVCharacterBase"));
    ensureMsgf(WeaponUser != nullptr,
        TEXT("[URVWeaponAttackComponent] Owner must implement IRVWeaponUser (e.g. ARVCharacterPlayer)"));

    // ForceEndAttack is subscribed by the owning Actor (ARVCharacterPlayer::BeginPlay).
    // Components do not self-subscribe to sibling component delegates.
}

UAnimInstance* URVWeaponAttackComponent::GetAnimInstance() const
{
    UAnimInstance* AnimInst = OwnerBase->GetMesh()->GetAnimInstance();
    ensureMsgf(IsValid(AnimInst),
        TEXT("[%s] URVWeaponAttackComponent: AnimInstance missing — check ABP assignment"),
        *GetNameSafe(OwnerBase));
    return AnimInst;
}

//--- Light Attack ------------------------------------------------------------

void URVWeaponAttackComponent::HandleLightAttackInput(bool bIsPlayerSprinting)
{
    if (!bIsComboActive)
    {
        if (!OwnerBase->CanAct(ERVCombatState::Attacking)) { return; }

        if (!OwnerBase->IsGrounded())
        {
            if (!bHasUsedJumpAttack) { StartJumpAttack(); }
            return;
        }

        if (bIsPlayerSprinting) { StartRunAttack(); return; }

        StartCombo();
        return;
    }

    if (bComboWindowOpen) { bHasComboInput = true; }
}

void URVWeaponAttackComponent::OpenComboWindow()  { bComboWindowOpen = true; }
void URVWeaponAttackComponent::CloseComboWindow() { bComboWindowOpen = false; }

void URVWeaponAttackComponent::TryChainNextCombo()
{
    if (!bHasComboInput) { EndCombo(); return; }

    const URVWeaponDataAsset* WeaponData = WeaponUser->GetCurrentWeaponData();
    if (!ensureMsgf(IsValid(WeaponData) && IsValid(WeaponData->CombatAnimData),
        TEXT("[%s] TryChainNextCombo: WeaponData or CombatAnimData not assigned"),
        *GetNameSafe(OwnerBase))) { EndCombo(); return; }

    UAnimInstance* AnimInst = GetAnimInstance();
    if (!IsValid(AnimInst)) { EndCombo(); return; }

    const UAnimMontage* CurrentMontage = AnimInst->GetCurrentActiveMontage();
    const int32 CurrentIndex = WeaponData->CombatAnimData->FindComboMontageIndex(CurrentMontage);
    if (CurrentIndex == INDEX_NONE) { EndCombo(); return; }

    UAnimMontage* NextMontage = WeaponData->CombatAnimData->GetComboMontage(CurrentIndex + 1);
    if (!IsValid(NextMontage)) { EndCombo(); return; }

    bHasComboInput = false;
    if (!ConsumeAttackStamina(NextMontage, WeaponData)) { EndCombo(); return; }

    PlayLightAttackMontage(NextMontage);
}

void URVWeaponAttackComponent::OnPlayerLanded()
{
    bHasUsedJumpAttack = false;
    if (!bIsJumpAttackActive) { return; }

    UAnimInstance* AnimInst = GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    const URVWeaponDataAsset* WeaponData = WeaponUser->GetCurrentWeaponData();
    if (!IsValid(WeaponData) || !IsValid(WeaponData->CombatAnimData)) { return; }

    UAnimMontage* Montage = WeaponData->CombatAnimData->JumpAttackMontage;
    if (!IsValid(Montage)) { return; }

    bIsJumpAttackLanding = true;

    UCharacterMovementComponent* MoveComp =
        Cast<ACharacter>(OwnerBase)->GetCharacterMovement();
    MoveComp->Velocity.X = 0.f;
    MoveComp->Velocity.Y = 0.f;

    AnimInst->Montage_JumpToSection(JumpAttackSection_Landing, Montage);
}

void URVWeaponAttackComponent::StartCombo()
{
    const URVWeaponDataAsset* WeaponData = WeaponUser->GetCurrentWeaponData();
    if (!ensureMsgf(IsValid(WeaponData) && IsValid(WeaponData->CombatAnimData),
        TEXT("[%s] StartCombo: WeaponData or CombatAnimData not assigned"),
        *GetNameSafe(OwnerBase))) { return; }

    UAnimMontage* FirstMontage = WeaponData->CombatAnimData->GetComboMontage(0);
    if (!ensureMsgf(IsValid(FirstMontage),
        TEXT("[%s] StartCombo: first combo montage not assigned"),
        *GetNameSafe(OwnerBase))) { return; }

    if (!ConsumeAttackStamina(FirstMontage, WeaponData)) { return; }

    bIsComboActive = true;
    OwnerBase->AddCombatState(ERVCombatState::Attacking);
    PlayLightAttackMontage(FirstMontage);
}

void URVWeaponAttackComponent::StartRunAttack()
{
    const URVWeaponDataAsset* WeaponData = WeaponUser->GetCurrentWeaponData();
    if (!ensureMsgf(IsValid(WeaponData) && IsValid(WeaponData->CombatAnimData),
        TEXT("[%s] StartRunAttack: WeaponData or CombatAnimData not assigned"),
        *GetNameSafe(OwnerBase))) { return; }

    UAnimMontage* Montage = WeaponData->CombatAnimData->RunAttackMontage;
    if (!ensureMsgf(IsValid(Montage),
        TEXT("[%s] StartRunAttack: RunAttackMontage not assigned"),
        *GetNameSafe(OwnerBase))) { return; }

    if (!ConsumeAttackStamina(Montage, WeaponData)) { return; }

    bIsComboActive = true;
    OwnerBase->AddCombatState(ERVCombatState::Attacking);
    PlayLightAttackMontage(Montage);
}

void URVWeaponAttackComponent::StartJumpAttack()
{
    const URVWeaponDataAsset* WeaponData = WeaponUser->GetCurrentWeaponData();
    if (!ensureMsgf(IsValid(WeaponData) && IsValid(WeaponData->CombatAnimData),
        TEXT("[%s] StartJumpAttack: WeaponData or CombatAnimData not assigned"),
        *GetNameSafe(OwnerBase))) { return; }

    UAnimMontage* Montage = WeaponData->CombatAnimData->JumpAttackMontage;
    if (!ensureMsgf(IsValid(Montage),
        TEXT("[%s] StartJumpAttack: JumpAttackMontage not assigned"),
        *GetNameSafe(OwnerBase))) { return; }

    if (!ConsumeAttackStamina(Montage, WeaponData)) { return; }

    bHasUsedJumpAttack  = true;
    bIsJumpAttackActive = true;
    bIsComboActive      = true;
    OwnerBase->AddCombatState(ERVCombatState::Attacking);

    UAnimInstance* AnimInst = GetAnimInstance();
    if (IsValid(AnimInst))
    {
        CachedRootMotionMode     = AnimInst->RootMotionMode;
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

    return Cost <= 0.f || OwnerBase->TryConsumeStamina(Cost);
}

void URVWeaponAttackComponent::PlayLightAttackMontage(UAnimMontage* InMontage)
{
    UAnimInstance* AnimInst = GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    AnimInst->Montage_Play(InMontage);

    FOnMontageBlendingOutStarted BlendingOutDelegate;
    BlendingOutDelegate.BindUObject(this, &URVWeaponAttackComponent::OnLightAttackMontageBlendingOut);
    AnimInst->Montage_SetBlendingOutDelegate(BlendingOutDelegate, InMontage);
}

void URVWeaponAttackComponent::EndCombo()
{
    if (bIsJumpAttackActive)
    {
        UAnimInstance* AnimInst = GetAnimInstance();
        // AnimInst may be null during rare destruction-time blend-out — skip without ensureMsgf.
        if (IsValid(AnimInst)) { AnimInst->RootMotionMode = CachedRootMotionMode; }
    }

    bIsComboActive       = false;
    bComboWindowOpen     = false;
    bHasComboInput       = false;
    bIsJumpAttackActive  = false;
    bIsJumpAttackLanding = false;

    OwnerBase->RemoveCombatState(ERVCombatState::Attacking);
}

void URVWeaponAttackComponent::OnLightAttackMontageBlendingOut(UAnimMontage*, bool bInterrupted)
{
    if (!bInterrupted && bIsComboActive) { EndCombo(); }
}

//--- Heavy Attack ------------------------------------------------------------

void URVWeaponAttackComponent::StartHeavyAttack()
{
    if (!OwnerBase->CanAct())                  { return; }
    if (!OwnerBase->IsGrounded())              { return; }
    if (OwnerBase->GetCurrentStamina() <= 0.f) { return; }

    const URVWeaponDataAsset* WeaponData = WeaponUser->GetCurrentWeaponData();
    if (!ensureMsgf(IsValid(WeaponData),
        TEXT("[%s] StartHeavyAttack: WeaponData not assigned"),
        *GetNameSafe(OwnerBase))) { return; }

    UAnimMontage* ChargeMontage = WeaponData->GetHeavyChargeMontage();
    if (!ensureMsgf(IsValid(ChargeMontage),
        TEXT("[%s] StartHeavyAttack: HeavyChargeMontage not assigned"),
        *GetNameSafe(OwnerBase))) { return; }

    OwnerBase->AddCombatState(ERVCombatState::HeavyCharging);
    bCanHeavyRelease = false;
    bPendingRelease  = false;
    bIsAutoRelease   = false;

    OwnerBase->ResetStaminaRegenDelay();

    UAnimInstance* AnimInst = GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    FOnMontageBlendingOutStarted ChargeBlendOut;
    ChargeBlendOut.BindUObject(this, &URVWeaponAttackComponent::OnChargeMontageBlendingOut);
    AnimInst->Montage_Play(ChargeMontage);
    AnimInst->Montage_SetBlendingOutDelegate(ChargeBlendOut, ChargeMontage);

    GetWorld()->GetTimerManager().SetTimer(
        ChargeAutoReleaseHandle, this, &URVWeaponAttackComponent::OnChargeAutoRelease,
        MaxChargeTime, false);
}

void URVWeaponAttackComponent::ReleaseHeavyAttack()
{
    if (!OwnerBase->HasCombatState(ERVCombatState::HeavyCharging)) { return; }
    if (!bCanHeavyRelease) { bPendingRelease = true; return; }
    ExecuteHeavyAttack();
}

void URVWeaponAttackComponent::SetHeavyAttackReady(bool bReady)
{
    bCanHeavyRelease = bReady;
    if (bReady && bPendingRelease) { bPendingRelease = false; ExecuteHeavyAttack(); }
}

void URVWeaponAttackComponent::ExecuteHeavyAttack()
{
    GetWorld()->GetTimerManager().ClearTimer(ChargeAutoReleaseHandle);

    const URVWeaponDataAsset* WeaponData = WeaponUser->GetCurrentWeaponData();
    if (!IsValid(WeaponData)) { EndHeavyAttack(); return; }

    UAnimInstance* AnimInst = GetAnimInstance();
    if (!IsValid(AnimInst)) { EndHeavyAttack(); return; }

    UAnimMontage* ReleaseMontage = WeaponData->GetHeavyAttackMontage(bIsAutoRelease);
    if (!IsValid(ReleaseMontage)) { EndHeavyAttack(); return; }

    const URVMontageStatData* StatData = ReleaseMontage->GetAssetUserData<URVMontageStatData>();
    const FRVAttackActionMultiplierRow* AttackStat = StatData ? StatData->GetStatRow() : nullptr;
    if (AttackStat && AttackStat->StaminaCostMultiplier > 0.f)
    {
        const FRVWeaponStatRow* WeaponStat = WeaponData->GetWeaponStatRow();
        if (WeaponStat)
        {
            OwnerBase->TryConsumeStamina(
                WeaponStat->BaseStaminaCost * AttackStat->StaminaCostMultiplier);
        }
    }

    bIsAutoRelease = false;
    OwnerBase->RemoveCombatState(ERVCombatState::HeavyCharging);
    OwnerBase->AddCombatState(ERVCombatState::HeavyAttacking);

    FOnMontageBlendingOutStarted ReleaseBlendOut;
    ReleaseBlendOut.BindUObject(this, &URVWeaponAttackComponent::OnReleaseMontageBlendingOut);
    AnimInst->Montage_Play(ReleaseMontage);
    AnimInst->Montage_SetBlendingOutDelegate(ReleaseBlendOut, ReleaseMontage);
}

void URVWeaponAttackComponent::EndHeavyAttack()
{
    if (!OwnerBase->HasCombatState(ERVCombatState::HeavyCharging | ERVCombatState::HeavyAttacking))
    {
        return;
    }

    UAnimInstance* AnimInst = GetAnimInstance();
    if (IsValid(AnimInst))
    {
        const URVWeaponDataAsset* WeaponData = WeaponUser->GetCurrentWeaponData();
        if (IsValid(WeaponData))
        {
            UAnimMontage* MontageToStop = OwnerBase->HasCombatState(ERVCombatState::HeavyCharging)
                ? WeaponData->GetHeavyChargeMontage()
                : AnimInst->GetCurrentActiveMontage();
            if (IsValid(MontageToStop)) { AnimInst->Montage_Stop(0.1f, MontageToStop); }
        }
    }

    OwnerBase->RemoveCombatState(ERVCombatState::HeavyCharging | ERVCombatState::HeavyAttacking);
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

void URVWeaponAttackComponent::OnChargeMontageBlendingOut(UAnimMontage*, bool)
{
    if (OwnerBase->HasCombatState(ERVCombatState::HeavyCharging)) { EndHeavyAttack(); }
}

void URVWeaponAttackComponent::OnReleaseMontageBlendingOut(UAnimMontage*, bool)
{
    EndHeavyAttack();
}

//--- Shared ------------------------------------------------------------------

void URVWeaponAttackComponent::ForceEndAttack()
{
    if (bIsComboActive)
    {
        UAnimInstance* AnimInst = GetAnimInstance();
        if (IsValid(AnimInst))
        {
            UAnimMontage* Current = AnimInst->GetCurrentActiveMontage();
            if (IsValid(Current)) { AnimInst->Montage_Stop(0.1f, Current); }
        }
        EndCombo();
    }

    if (OwnerBase->HasCombatState(ERVCombatState::HeavyCharging | ERVCombatState::HeavyAttacking))
    {
        EndHeavyAttack();
    }
}
