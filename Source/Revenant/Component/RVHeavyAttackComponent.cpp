#include "Component/RVHeavyAttackComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Data/RVWeaponDataAsset.h"
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

    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData) || !IsValid(WeaponData->GetHeavyChargeMontage())) { return; }

    if (!AttributeComponent->ConsumeStamina(WeaponData->HeavyAttackStaminaCost)) { return; }

    // AddState broadcasts OnStateChanged — SprintComponent self-terminates from that.
    // No explicit EndSprint call needed here.
    CombatStateComponent->AddState(ERVCombatState::HeavyCharging);
    bCanHeavyRelease = false;
    bPendingRelease  = false;
    bIsAutoRelease   = false;
    AttributeComponent->PauseStaminaRegen();

    UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInstance)) { return; }

    UAnimMontage* ChargeMontage = WeaponData->GetHeavyChargeMontage();

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

    UAnimMontage* ReleaseMontage = WeaponData->GetHeavyAttackMontage();
    if (!IsValid(ReleaseMontage)) { EndHeavyAttack(); return; }

    const ERVHeavyAttackTier Tier = bIsAutoRelease
        ? ERVHeavyAttackTier::AutoRelease
        : ERVHeavyAttackTier::Manual;
    CombatStateComponent->SetHeavyAttackTier(Tier);
    bIsAutoRelease = false;

    // Remove HeavyCharging before playing release montage.
    // OnChargeMontageBlendingOut checks HeavyCharging to detect external interruption —
    // removing it here signals that this stop is intentional.
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
                : WeaponData->GetHeavyAttackMontage();
            AnimInstance->Montage_Stop(0.1f, MontageToStop);
        }
    }

    CombatStateComponent->RemoveState(ERVCombatState::HeavyCharging | ERVCombatState::HeavyAttacking);
    bCanHeavyRelease = false;
    bPendingRelease  = false;
    bIsAutoRelease   = false;
    GetWorld()->GetTimerManager().ClearTimer(ChargeAutoReleaseHandle);
    AttributeComponent->ResumeStaminaRegen();
}

void URVHeavyAttackComponent::OnChargeAutoRelease()
{
    UE_LOG(LogTemp, Log, TEXT("[%s] URVHeavyAttackComponent: OnChargeAutoRelease fired"), *GetNameSafe(GetOwner()));
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