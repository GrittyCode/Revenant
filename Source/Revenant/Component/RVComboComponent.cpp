#include "Component/RVComboComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Data/RVWeaponDataAsset.h"
#include "Data/RVPlayerCombatAnimDataAsset.h"
#include "Data/RVAttackActionMultiplierRow.h"
#include "Data/RVWeaponStatRow.h"
#include "Data/RVMontageStatData.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"

URVComboComponent::URVComboComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URVComboComponent::BeginPlay()
{
    Super::BeginPlay();
}

void URVComboComponent::InitReferences(
    ACharacter* InOwnerCharacter,
    URVCombatStateComponent* InCombatStateComponent,
    URVAttributeComponent* InAttributeComponent,
    URVEquipmentComponent* InEquipmentComponent)
{
    OwnerCharacter       = InOwnerCharacter;
    CombatStateComponent = InCombatStateComponent;
    AttributeComponent   = InAttributeComponent;
    EquipmentComponent   = InEquipmentComponent;

    CombatStateComponent->OnForceEnd.AddUObject(this, &URVComboComponent::ForceEndCombo);
}

//--- Input -------------------------------------------------------------------

void URVComboComponent::HandleComboInput()
{
    if (!bIsComboActive)
    {
        if (!CombatStateComponent->IsGrounded()) { return; }
        if (!CombatStateComponent->CheckAvailableState(ERVCombatState::Attacking)) { return; }
        StartCombo();
        return;
    }

    if (bComboWindowOpen) { bHasComboInput = true; }
}

//--- Combo Window ------------------------------------------------------------

void URVComboComponent::OpenComboWindow()
{
    bComboWindowOpen = true;
}

void URVComboComponent::CloseComboWindow()
{
    bComboWindowOpen = false;
}

//--- Chain -------------------------------------------------------------------

void URVComboComponent::TryChainNextCombo()
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

    if (!ConsumeComboStamina(NextMontage, WeaponData)) { EndCombo(); return; }

    PlayComboMontage(NextMontage);
}

//--- Internal ----------------------------------------------------------------

void URVComboComponent::StartCombo()
{
    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData) || !IsValid(WeaponData->CombatAnimData)) { return; }

    UAnimMontage* FirstMontage = WeaponData->CombatAnimData->GetComboMontage(0);
    if (!IsValid(FirstMontage)) { return; }

    if (!ConsumeComboStamina(FirstMontage, WeaponData)) { return; }

    bIsComboActive = true;
    OnComboStarted.Broadcast();

    PlayComboMontage(FirstMontage);
}

bool URVComboComponent::ConsumeComboStamina(UAnimMontage* InMontage, const URVWeaponDataAsset* InWeaponData)
{
    const URVMontageStatData* StatData = InMontage->GetAssetUserData<URVMontageStatData>();
    const FRVAttackActionMultiplierRow* AttackStat = StatData ? StatData->GetStatRow() : nullptr;
    if (!AttackStat || AttackStat->StaminaCostMultiplier <= 0.f) { return true; }

    const FRVWeaponStatRow* WeaponStat = InWeaponData->GetWeaponStatRow();
    const float StaminaCost = WeaponStat ? WeaponStat->BaseStaminaCost * AttackStat->StaminaCostMultiplier : 0.f;

    return StaminaCost <= 0.f || AttributeComponent->ConsumeStamina(StaminaCost);
}

void URVComboComponent::PlayComboMontage(UAnimMontage* InMontage)
{
    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    AnimInst->Montage_Play(InMontage);

    FOnMontageBlendingOutStarted BlendingOutDelegate;
    BlendingOutDelegate.BindUObject(this, &URVComboComponent::OnComboMontageBlendingOut);
    AnimInst->Montage_SetBlendingOutDelegate(BlendingOutDelegate, InMontage);
}

void URVComboComponent::EndCombo()
{
    bIsComboActive   = false;
    bComboWindowOpen = false;
    bHasComboInput   = false;

    OnComboEnded.Broadcast();
}

void URVComboComponent::ForceEndCombo()
{
    if (!bIsComboActive) { return; }

    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (IsValid(AnimInst))
    {
        UAnimMontage* CurrentMontage = AnimInst->GetCurrentActiveMontage();
        if (IsValid(CurrentMontage))
        {
            AnimInst->Montage_Stop(0.1f, CurrentMontage);
        }
    }

    EndCombo();
}

//--- Callbacks ---------------------------------------------------------------

void URVComboComponent::OnComboMontageBlendingOut(UAnimMontage* /*Montage*/, bool bInterrupted)
{
    if (!bInterrupted && bIsComboActive)
    {
        EndCombo();
    }
}