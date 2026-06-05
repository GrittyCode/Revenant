#include "Component/Combat/RVWeaponAttackComponent.h"
#include "Component/Attribute/RVStaminaComponent.h"
#include "Component/Utility/RVEquipmentComponent.h"
#include "Character/Base/RVCharacterBase.h"
#include "Data/Asset/RVWeaponDataAsset.h"
#include "Data/Asset/RVPlayerCombatAnimDataAsset.h"
#include "Data/Asset/RVMontageStatData.h"
#include "Data/Row/RVAttackActionMultiplierRow.h"
#include "Data/Row/RVWeaponStatRow.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"

static const FName JumpAttackSection_Landing = FName("Landing");

URVWeaponAttackComponent::URVWeaponAttackComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URVWeaponAttackComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerBase = Cast<ARVCharacterBase>(GetOwner());
    ensureMsgf(IsValid(OwnerBase),
        TEXT("[URVWeaponAttackComponent] Owner must be ARVCharacterBase"));
}

void URVWeaponAttackComponent::Init(
    URVStaminaComponent* InStamina, URVEquipmentComponent* InEquipment)
{
    ensureMsgf(IsValid(InStamina),   TEXT("[URVWeaponAttackComponent] Init: StaminaComponent is null"));
    ensureMsgf(IsValid(InEquipment), TEXT("[URVWeaponAttackComponent] Init: EquipmentComponent is null"));
    StaminaComponent   = InStamina;
    EquipmentComponent = InEquipment;
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
    if (!bIsLightAttackActive)
    {
        if (!OwnerBase->CanAct()) { return; }

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

    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
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
    if (!ConsumeAttackStamina(NextMontage)) { EndCombo(); return; }

    PlayLightAttackMontage(NextMontage);
}

void URVWeaponAttackComponent::OnPlayerLanded()
{
    bHasUsedJumpAttack = false;
    if (!bIsJumpAttackActive) { return; }

    UAnimInstance* AnimInst = GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData) || !IsValid(WeaponData->CombatAnimData)) { return; }

    UAnimMontage* Montage = WeaponData->CombatAnimData->JumpAttackMontage;
    if (!IsValid(Montage)) { return; }

    bIsJumpAttackLanding = true;

    UCharacterMovementComponent* MoveComp = OwnerBase->GetCharacterMovement();
    MoveComp->Velocity = FVector(0.f, 0.f, MoveComp->Velocity.Z);

    AnimInst->Montage_JumpToSection(JumpAttackSection_Landing, Montage);
}

void URVWeaponAttackComponent::StartCombo()
{
    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!ensureMsgf(IsValid(WeaponData) && IsValid(WeaponData->CombatAnimData),
        TEXT("[%s] StartCombo: WeaponData or CombatAnimData not assigned"),
        *GetNameSafe(OwnerBase))) { return; }

    UAnimMontage* FirstMontage = WeaponData->CombatAnimData->GetComboMontage(0);
    if (!ensureMsgf(IsValid(FirstMontage),
        TEXT("[%s] StartCombo: first combo montage not assigned"),
        *GetNameSafe(OwnerBase))) { return; }

    if (!ConsumeAttackStamina(FirstMontage)) { return; }

    bIsLightAttackActive = true;
    OwnerBase->AddCombatState(ERVCombatState::Attacking);
    PlayLightAttackMontage(FirstMontage);
}

void URVWeaponAttackComponent::StartRunAttack()
{
    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!ensureMsgf(IsValid(WeaponData) && IsValid(WeaponData->CombatAnimData),
        TEXT("[%s] StartRunAttack: WeaponData or CombatAnimData not assigned"),
        *GetNameSafe(OwnerBase))) { return; }

    UAnimMontage* Montage = WeaponData->CombatAnimData->RunAttackMontage;
    if (!ensureMsgf(IsValid(Montage),
        TEXT("[%s] StartRunAttack: RunAttackMontage not assigned"),
        *GetNameSafe(OwnerBase))) { return; }

    if (!ConsumeAttackStamina(Montage)) { return; }

    bIsLightAttackActive = true;
    OwnerBase->AddCombatState(ERVCombatState::Attacking);
    PlayLightAttackMontage(Montage);
}

void URVWeaponAttackComponent::StartJumpAttack()
{
    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!ensureMsgf(IsValid(WeaponData) && IsValid(WeaponData->CombatAnimData),
        TEXT("[%s] StartJumpAttack: WeaponData or CombatAnimData not assigned"),
        *GetNameSafe(OwnerBase))) { return; }

    UAnimMontage* Montage = WeaponData->CombatAnimData->JumpAttackMontage;
    if (!ensureMsgf(IsValid(Montage),
        TEXT("[%s] StartJumpAttack: JumpAttackMontage not assigned"),
        *GetNameSafe(OwnerBase))) { return; }

    if (!ConsumeAttackStamina(Montage)) { return; }

    bHasUsedJumpAttack   = true;
    bIsJumpAttackActive  = true;
    bIsLightAttackActive = true;
    OwnerBase->AddCombatState(ERVCombatState::Attacking);

    UAnimInstance* AnimInst = GetAnimInstance();
    if (IsValid(AnimInst))
    {
        CachedRootMotionMode     = AnimInst->RootMotionMode;
        AnimInst->RootMotionMode = ERootMotionMode::IgnoreRootMotion;
    }

    PlayLightAttackMontage(Montage);
}

bool URVWeaponAttackComponent::ConsumeAttackStamina(UAnimMontage* InMontage)
{
    const URVMontageStatData* StatData = InMontage->GetAssetUserData<URVMontageStatData>();
    const FRVAttackActionMultiplierRow* AttackStat = StatData ? StatData->GetStatRow() : nullptr;
    if (!AttackStat || AttackStat->StaminaCostMultiplier <= 0.f) { return true; }

    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    const FRVWeaponStatRow*   WeaponStat = IsValid(WeaponData) ? WeaponData->GetWeaponStatRow() : nullptr;
    const float Cost = WeaponStat ? WeaponStat->BaseStaminaCost * AttackStat->StaminaCostMultiplier : 0.f;

    return Cost <= 0.f || StaminaComponent->ConsumeStamina(Cost);
}

void URVWeaponAttackComponent::PlayLightAttackMontage(UAnimMontage* InMontage)
{
	UAnimInstance* AnimInst = GetAnimInstance();
	if (!IsValid(AnimInst)) { return; }

	ActiveLightMontage = InMontage;
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
		if (IsValid(AnimInst)) { AnimInst->RootMotionMode = CachedRootMotionMode; }
	}

	bIsLightAttackActive = false;
	bComboWindowOpen     = false;
	bHasComboInput       = false;
	bIsJumpAttackActive  = false;
	bIsJumpAttackLanding = false;
	ActiveLightMontage   = nullptr;

	OwnerBase->RemoveCombatState(ERVCombatState::Attacking);
}

void URVWeaponAttackComponent::OnLightAttackMontageBlendingOut(UAnimMontage* Montage, bool)
{
	if (bIsLightAttackActive && Montage == ActiveLightMontage) { EndCombo(); }
}

//--- Heavy Attack ------------------------------------------------------------

void URVWeaponAttackComponent::StartHeavyAttack()
{
    if (!OwnerBase->CanAct())                          { return; }
    if (!OwnerBase->IsGrounded())                      { return; }
    if (StaminaComponent->GetCurrentStamina() <= 0.f) { return; }

    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!ensureMsgf(IsValid(WeaponData),
        TEXT("[%s] StartHeavyAttack: WeaponData not assigned"),
        *GetNameSafe(OwnerBase))) { return; }

    UAnimMontage* ChargeMontage = WeaponData->GetHeavyChargeMontage();
    if (!ensureMsgf(IsValid(ChargeMontage),
        TEXT("[%s] StartHeavyAttack: HeavyChargeMontage not assigned"),
        *GetNameSafe(OwnerBase))) { return; }

    OwnerBase->AddCombatState(ERVCombatState::Attacking);
    HeavyPhase = EHeavyPhase::Charging;

    bCanHeavyRelease = false;
    bPendingRelease  = false;
    bIsAutoRelease   = false;

    StaminaComponent->ResetStaminaRegenDelay();

    UAnimInstance* AnimInst = GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    FOnMontageBlendingOutStarted ChargeBlendOut;
    ChargeBlendOut.BindUObject(this, &URVWeaponAttackComponent::OnChargeMontageBlendingOut);
    AnimInst->Montage_Play(ChargeMontage);
    AnimInst->Montage_SetBlendingOutDelegate(ChargeBlendOut, ChargeMontage);

    const FRVWeaponStatRow* WeaponStat = WeaponData->GetWeaponStatRow();
    const float ChargeTime = WeaponStat ? WeaponStat->MaxChargeTime : 1.5f;
    GetWorld()->GetTimerManager().SetTimer(
        ChargeAutoReleaseHandle, this, &URVWeaponAttackComponent::OnChargeAutoRelease,
        ChargeTime, false);
}

void URVWeaponAttackComponent::ReleaseHeavyAttack()
{
    if (HeavyPhase != EHeavyPhase::Charging) { return; }
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

    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
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
            StaminaComponent->ConsumeStamina(
                WeaponStat->BaseStaminaCost * AttackStat->StaminaCostMultiplier);
        }
    }

    bIsAutoRelease = false;
    HeavyPhase = EHeavyPhase::Releasing;

    FOnMontageBlendingOutStarted ReleaseBlendOut;
    ReleaseBlendOut.BindUObject(this, &URVWeaponAttackComponent::OnReleaseMontageBlendingOut);
    AnimInst->Montage_Play(ReleaseMontage);
    AnimInst->Montage_SetBlendingOutDelegate(ReleaseBlendOut, ReleaseMontage);
}

void URVWeaponAttackComponent::EndHeavyAttack()
{
    if (HeavyPhase == EHeavyPhase::None) { return; }

    UAnimInstance* AnimInst = GetAnimInstance();
    if (IsValid(AnimInst))
    {
        const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
        if (IsValid(WeaponData))
        {
            UAnimMontage* MontageToStop = (HeavyPhase == EHeavyPhase::Charging)
                ? WeaponData->GetHeavyChargeMontage()
                : AnimInst->GetCurrentActiveMontage();
            if (IsValid(MontageToStop)) { AnimInst->Montage_Stop(0.1f, MontageToStop); }
        }
    }

    OwnerBase->RemoveCombatState(ERVCombatState::Attacking);
    HeavyPhase = EHeavyPhase::None;

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
    if (HeavyPhase == EHeavyPhase::Charging) { EndHeavyAttack(); }
}

void URVWeaponAttackComponent::OnReleaseMontageBlendingOut(UAnimMontage*, bool)
{
    EndHeavyAttack();
}

//--- Shared ------------------------------------------------------------------

void URVWeaponAttackComponent::ForceEndAttack()
{
    if (bIsLightAttackActive)
    {
        UAnimInstance* AnimInst = GetAnimInstance();
        if (IsValid(AnimInst))
        {
            UAnimMontage* Current = AnimInst->GetCurrentActiveMontage();
            if (IsValid(Current)) { AnimInst->Montage_Stop(0.1f, Current); }
        }
        EndCombo();
    }

    if (HeavyPhase != EHeavyPhase::None)
    {
        EndHeavyAttack();
    }
}
