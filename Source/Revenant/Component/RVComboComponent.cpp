// Source/Revenant/Component/RVComboComponent.cpp
#include "Component/RVComboComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Component/RVCombatComponent.h"
#include "Component/RVAttributeComponent.h"
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
    EquipmentComponent = Owner->FindComponentByClass<URVEquipmentComponent>();
    CombatComponent    = Owner->FindComponentByClass<URVCombatComponent>();
    AttributeComponent = Owner->FindComponentByClass<URVAttributeComponent>();
}

// --- Public API --------------------------------------------------------------

void URVComboComponent::HandleComboInput()
{
    if (!IsValid(CombatComponent)) { return; }

    if (!bIsComboActive)
    {
        // Not attacking -- start fresh if no blocking state (Attacking and Guarding excluded)
        if (!CombatComponent->CanPerformAction(ERVCombatState::Attacking | ERVCombatState::Guarding))
        {
	        return;
        }

        StartCombo();
    }
    else
    {
        // Already in a combo -- buffer the next hit
        if (!IsValid(EquipmentComponent)) { return; }

        URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
        if (!IsValid(WeaponData)) { return; }

        if (ComboCount < WeaponData->MaxComboCount)
        {
            bHasComboInput = true;
        }
    }
}

void URVComboComponent::TryAdvanceCombo()
{
    if (!bIsComboActive) { return; }

    if (bHasComboInput)
    {
        bHasComboInput = false;
        ++ComboCount;
        PlayComboSection();
    }
    else
    {
        // Window opened but no input -- combo ends after this section
        EndCombo();
    }
}

// --- Internal ----------------------------------------------------------------

void URVComboComponent::StartCombo()
{
    if (!IsValid(EquipmentComponent) || !IsValid(AttributeComponent)) { return; }

    URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData) || !IsValid(WeaponData->AttackMontage)) { return; }

    if (!AttributeComponent->ConsumeStamina(WeaponData->AttackStaminaCost)) { return; }

    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!IsValid(OwnerChar)) { return; }

    UAnimInstance* AnimInst = OwnerChar->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    bIsComboActive = true;
    ComboCount     = 1;
    bHasComboInput = false;

    if (IsValid(CombatComponent))
    {
        CombatComponent->SetAttacking(true);
    }

    if (IsValid(AttributeComponent))
    {
        AttributeComponent->PauseStaminaRegen();
    }

    FOnMontageBlendingOutStarted BlendOutDelegate;
    BlendOutDelegate.BindUObject(this, &URVComboComponent::OnComboMontageBlendingOut);

    AnimInst->Montage_Play(WeaponData->AttackMontage);
    AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, WeaponData->AttackMontage);

    PlayComboSection();
}

void URVComboComponent::EndCombo()
{
    bIsComboActive = false;
    bHasComboInput = false;
    ComboCount     = 0;

    if (IsValid(CombatComponent))
    {
        CombatComponent->SetAttacking(false);
    }

    if (IsValid(AttributeComponent))
    {
        AttributeComponent->ResumeStaminaRegen();
    }
}

void URVComboComponent::PlayComboSection()
{
    if (!IsValid(EquipmentComponent)) { return; }

    URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData) || !IsValid(WeaponData->AttackMontage)) { return; }

    if (!WeaponData->ComboSectionNames.IsValidIndex(ComboCount - 1)) { return; }

    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!IsValid(OwnerChar)) { return; }

    UAnimInstance* AnimInst = OwnerChar->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    const FName SectionName = WeaponData->ComboSectionNames[ComboCount - 1];
    AnimInst->Montage_JumpToSection(SectionName, WeaponData->AttackMontage);
}

void URVComboComponent::OnComboMontageBlendingOut(UAnimMontage* /*InMontage*/, bool /*bInterrupted*/)
{
    EndCombo();
}