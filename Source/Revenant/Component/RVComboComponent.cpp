#include "Component/RVComboComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Data/RVWeaponDataAsset.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"

URVComboComponent::URVComboComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URVComboComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    EquipmentComponent   = Owner->FindComponentByClass<URVEquipmentComponent>();
    AttributeComponent   = Owner->FindComponentByClass<URVAttributeComponent>();
    CombatStateComponent = Owner->FindComponentByClass<URVCombatStateComponent>();
    OwnerCharacter       = Cast<ACharacter>(Owner);

    ensureMsgf(IsValid(EquipmentComponent),   TEXT("[%s] URVEquipmentComponent missing — ComboComponent requires ARVCharacterBase"),   *GetNameSafe(Owner));
    ensureMsgf(IsValid(AttributeComponent),   TEXT("[%s] URVAttributeComponent missing — ComboComponent requires ARVCharacterBase"),   *GetNameSafe(Owner));
    ensureMsgf(IsValid(CombatStateComponent), TEXT("[%s] URVCombatStateComponent missing — ComboComponent requires ARVCharacterBase"), *GetNameSafe(Owner));
    ensureMsgf(IsValid(OwnerCharacter),       TEXT("[%s] Owner is not ACharacter — ComboComponent requires ARVCharacterBase"),         *GetNameSafe(Owner));

    // Subscribe to CombatStateComponent's force-end broadcast.
    // When ForceEndAllActions fires (e.g. hit reaction), ComboComponent cleans up its own state.
    CombatStateComponent->OnForceEnd.AddUObject(this, &URVComboComponent::ForceEndCombo);
}

// --- Public API --------------------------------------------------------------

void URVComboComponent::HandleComboInput()
{
    if (!bIsComboActive)
    {
        if (!CombatStateComponent->IsGrounded()) { return; }

        // Attacking and Guarding are coexistable:
        if (!CombatStateComponent->CheckAvailableState(ERVCombatState::Attacking | ERVCombatState::Guarding)) { return; }

        StartCombo();
    }
    else
    {
        // Only accept continuation input during the explicit combo window.
        // Input outside the window is discarded — enforces timing requirement.
        if (!bComboWindowOpen) { return; }

        URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
        if (!IsValid(WeaponData)) { return; }

        if (ComboCount < WeaponData->GetMaxComboCount())
        {
            bHasComboInput = true;
        }
    }
}

void URVComboComponent::OpenComboWindow()
{
    if (!bIsComboActive) { return; }
    bComboWindowOpen = true;
}

void URVComboComponent::CloseComboWindow()
{
    if (!bIsComboActive) { return; }

    bComboWindowOpen = false;
    TryAdvanceCombo();
}

// --- Internal ----------------------------------------------------------------

void URVComboComponent::StartCombo()
{
    URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData) || !IsValid(WeaponData->GetAttackMontage())) { return; }

    // Stamina cost check — gates combo entry
    if (!AttributeComponent->ConsumeStamina(WeaponData->AttackStaminaCost)) { return; }

    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    bIsComboActive = true;
    ComboCount     = 1;
    bHasComboInput = false;

    AttributeComponent->PauseStaminaRegen();

    // Notify CombatStateComponent: sets Attacking bit, clears Guarding if active
    OnComboStarted.Broadcast();

    UAnimMontage* AttackMontage = WeaponData->GetAttackMontage();

    AnimInst->Montage_Play(AttackMontage);

    FOnMontageEnded MontageEndedDelegate;
    MontageEndedDelegate.BindUObject(this, &URVComboComponent::OnComboMontageEnded);
    AnimInst->Montage_SetEndDelegate(MontageEndedDelegate, AttackMontage);

    PlayComboSection();
}

void URVComboComponent::EndCombo()
{
    bIsComboActive   = false;
    bHasComboInput   = false;
    bComboWindowOpen = false;
    ComboCount       = 0;

    AttributeComponent->ResumeStaminaRegen();

    // Notify CombatStateComponent: clears Attacking bit
    OnComboEnded.Broadcast();
}

void URVComboComponent::TryAdvanceCombo()
{
    if (!bIsComboActive) { return; }

    if (bHasComboInput)
    {
        // Consume stamina for the next hit before advancing.
        // If stamina is insufficient the combo ends naturally here.
        URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
        if (!IsValid(WeaponData)) { return; }

        if (!AttributeComponent->ConsumeStamina(WeaponData->AttackStaminaCost))
        {
            // Out of stamina — treat as if no input was buffered
            bHasComboInput = false;
            EndCombo();
            return;
        }

        bHasComboInput = false;
        ++ComboCount;
        PlayComboSection();
    }
}

void URVComboComponent::ForceEndCombo()
{
    if (!bIsComboActive) { return; }

    // Stop the attack montage so visual state matches logical state.
    URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (IsValid(AnimInst) && IsValid(WeaponData))
    {
        AnimInst->Montage_Stop(0.1f, WeaponData->GetAttackMontage());
    }

    // EndCombo broadcasts OnComboEnded → CombatStateComponent clears Attacking bit.
    EndCombo();
}

void URVComboComponent::PlayComboSection()
{
    URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData) || !IsValid(WeaponData->GetAttackMontage())) { return; }

    const TArray<FName>& SectionNames = WeaponData->GetComboSectionNames();
    if (!SectionNames.IsValidIndex(ComboCount - 1)) { return; }

    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    const FName SectionName = SectionNames[ComboCount - 1];
    AnimInst->Montage_JumpToSection(SectionName, WeaponData->GetAttackMontage());
}

void URVComboComponent::OnComboMontageEnded(UAnimMontage* /*InMontage*/, bool /*bInterrupted*/)
{
    EndCombo();
}