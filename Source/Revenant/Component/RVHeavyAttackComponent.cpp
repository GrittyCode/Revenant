#include "Component/RVHeavyAttackComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Data/RVWeaponDataAsset.h"
#include "Data/RVWeaponStatRow.h"
#include "Data/RVMontageStatData.h"
#include "Data/RVAttackActionMultiplierRow.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"

URVHeavyAttackComponent::URVHeavyAttackComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URVHeavyAttackComponent::BeginPlay()
{
    Super::BeginPlay();
}

void URVHeavyAttackComponent::InitReferences(
    ACharacter* InOwnerCharacter,
    URVCombatStateComponent* InCombatStateComponent,
    URVAttributeComponent* InAttributeComponent,
    URVEquipmentComponent* InEquipmentComponent)
{
    OwnerCharacter       = InOwnerCharacter;
    CombatStateComponent = InCombatStateComponent;
    AttributeComponent   = InAttributeComponent;
    EquipmentComponent   = InEquipmentComponent;
}

void URVHeavyAttackComponent::StartHeavyAttack()
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

    // Charge has no per-frame stamina cost, so ConsumeStamina won't reset the clock.
    // Reset manually so regen doesn't tick during the charge window.
    AttributeComponent->ResetStaminaRegenDelay();

    UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInstance)) { return; }

    FOnMontageBlendingOutStarted ChargeBlendOutDelegate;
    ChargeBlendOutDelegate.BindUObject(this, &URVHeavyAttackComponent::OnChargeMontageBlendingOut);

    AnimInstance->Montage_Play(ChargeMontage);
    AnimInstance->Montage_SetBlendingOutDelegate(ChargeBlendOutDelegate, ChargeMontage);

    GetWorld()->GetTimerManager().SetTimer(
        ChargeAutoReleaseHandle,
        this,
        &URVHeavyAttackComponent::OnChargeAutoRelease,
        MaxChargeTime,
        false
    );
}

void URVHeavyAttackComponent::ReleaseHeavyAttack()
{
    if (!CombatStateComponent->HasState(ERVCombatState::HeavyCharging)) { return; }

    if (!bCanHeavyRelease)
    {
        bPendingRelease = true;
        return;
    }

    ExecuteHeavyAttack();
}

void URVHeavyAttackComponent::SetHeavyAttackReady(bool bReady)
{
    bCanHeavyRelease = bReady;

    if (bReady && bPendingRelease)
    {
        bPendingRelease = false;
        ExecuteHeavyAttack();
    }
}

void URVHeavyAttackComponent::ForceEndHeavyAttack()
{
    if (!CombatStateComponent->HasState(ERVCombatState::HeavyCharging | ERVCombatState::HeavyAttacking)) { return; }
    EndHeavyAttack();
}

void URVHeavyAttackComponent::ExecuteHeavyAttack()
{
    GetWorld()->GetTimerManager().ClearTimer(ChargeAutoReleaseHandle);

    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData)) { EndHeavyAttack(); return; }

    UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInstance)) { EndHeavyAttack(); return; }

    UAnimMontage* ReleaseMontage = WeaponData->GetHeavyAttackMontage(bIsAutoRelease);
    if (!IsValid(ReleaseMontage)) { EndHeavyAttack(); return; }

    // Consume stamina at release — also resets the regen delay clock via ConsumeStamina.
    const URVMontageStatData* StatData = ReleaseMontage->GetAssetUserData<URVMontageStatData>();
    const FRVAttackActionMultiplierRow* AttackStat = StatData ? StatData->GetStatRow() : nullptr;
    if (AttackStat && AttackStat->StaminaCostMultiplier > 0.f)
    {
        const FRVWeaponStatRow* WeaponStat = WeaponData->GetWeaponStatRow();
        if (WeaponStat)
        {
            AttributeComponent->ConsumeStamina(WeaponStat->BaseStaminaCost * AttackStat->StaminaCostMultiplier);
        }
    }

    bIsAutoRelease = false;

    CombatStateComponent->RemoveState(ERVCombatState::HeavyCharging);
    CombatStateComponent->AddState(ERVCombatState::HeavyAttacking);

    FOnMontageBlendingOutStarted ReleaseBlendOutDelegate;
    ReleaseBlendOutDelegate.BindUObject(this, &URVHeavyAttackComponent::OnReleaseMontageBlendingOut);

    AnimInstance->Montage_Play(ReleaseMontage);
    AnimInstance->Montage_SetBlendingOutDelegate(ReleaseBlendOutDelegate, ReleaseMontage);
}

void URVHeavyAttackComponent::EndHeavyAttack()
{
    if (!CombatStateComponent->HasState(ERVCombatState::HeavyCharging | ERVCombatState::HeavyAttacking)) { return; }

    UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (IsValid(AnimInstance))
    {
        const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
        if (IsValid(WeaponData))
        {
            UAnimMontage* MontageToStop = CombatStateComponent->HasState(ERVCombatState::HeavyCharging)
                ? WeaponData->GetHeavyChargeMontage()
                : AnimInstance->GetCurrentActiveMontage();

            if (IsValid(MontageToStop))
            {
                AnimInstance->Montage_Stop(0.1f, MontageToStop);
            }
        }
    }

    CombatStateComponent->RemoveState(ERVCombatState::HeavyCharging | ERVCombatState::HeavyAttacking);
    bCanHeavyRelease = false;
    bPendingRelease  = false;
    bIsAutoRelease   = false;
    GetWorld()->GetTimerManager().ClearTimer(ChargeAutoReleaseHandle);
}

void URVHeavyAttackComponent::OnChargeAutoRelease()
{
    bIsAutoRelease = true;
    ReleaseHeavyAttack();
}

void URVHeavyAttackComponent::OnChargeMontageBlendingOut(UAnimMontage* /*InMontage*/, bool /*bInterrupted*/)
{
    if (CombatStateComponent->HasState(ERVCombatState::HeavyCharging))
    {
        EndHeavyAttack();
    }
}

void URVHeavyAttackComponent::OnReleaseMontageBlendingOut(UAnimMontage* /*InMontage*/, bool /*bInterrupted*/)
{
    EndHeavyAttack();
}