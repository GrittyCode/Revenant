// Source/Revenant/Component/RVComboComponent.cpp
#include "Component/RVComboComponent.h"
#include "Component/RVEquipmentComponent.h"
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
    AttributeComponent = Owner->FindComponentByClass<URVAttributeComponent>();

    // URVComboComponent must only be placed on ARVCharacterBase subclasses.
    ensureMsgf(IsValid(EquipmentComponent), TEXT("[%s] URVEquipmentComponent missing — ComboComponent requires ARVCharacterBase"), *GetNameSafe(Owner));
    ensureMsgf(IsValid(AttributeComponent), TEXT("[%s] URVAttributeComponent missing — ComboComponent requires ARVCharacterBase"), *GetNameSafe(Owner));
}

// --- Public API --------------------------------------------------------------

void URVComboComponent::HandleComboInput()
{
    if (!bIsComboActive)
    {
        // Gate checks are the caller's responsibility (URVCombatComponent::TryStartCombo)
        StartCombo();
    }
    else
    {
        // Already in a combo — buffer the next hit
        URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
        if (!IsValid(WeaponData)) { return; }

        if (ComboCount < WeaponData->GetMaxComboCount())
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
        // Window opened but no input — combo ends after this section
        EndCombo();
    }
}

// --- Internal ----------------------------------------------------------------

void URVComboComponent::StartCombo()
{
    URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData) || !IsValid(WeaponData->GetAttackMontage())) { return; }

    // Stamina cost check — gates combo entry without consuming regen control
    if (!AttributeComponent->ConsumeStamina(WeaponData->AttackStaminaCost)) { return; }

    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!IsValid(OwnerChar)) { return; }

    UAnimInstance* AnimInst = OwnerChar->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    bIsComboActive = true;
    ComboCount     = 1;
    bHasComboInput = false;

    AttributeComponent->PauseStaminaRegen();

    // Notify CombatComponent: sets bIsAttacking, clears bIsGuarding if active
    OnComboStarted.Broadcast();

    UAnimMontage* AttackMontage = WeaponData->GetAttackMontage();

    FOnMontageBlendingOutStarted BlendOutDelegate;
    BlendOutDelegate.BindUObject(this, &URVComboComponent::OnComboMontageBlendingOut);

    AnimInst->Montage_Play(AttackMontage);
    AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, AttackMontage);

    PlayComboSection();
}

void URVComboComponent::EndCombo()
{
    bIsComboActive = false;
    bHasComboInput = false;
    ComboCount     = 0;

    AttributeComponent->ResumeStaminaRegen();

    // Notify CombatComponent: clears bIsAttacking
    OnComboEnded.Broadcast();
}

void URVComboComponent::PlayComboSection()
{
    URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData) || !IsValid(WeaponData->GetAttackMontage())) { return; }

    const TArray<FName>& SectionNames = WeaponData->GetComboSectionNames();
    if (!SectionNames.IsValidIndex(ComboCount - 1)) { return; }

    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!IsValid(OwnerChar)) { return; }

    UAnimInstance* AnimInst = OwnerChar->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    const FName SectionName = SectionNames[ComboCount - 1];
    AnimInst->Montage_JumpToSection(SectionName, WeaponData->GetAttackMontage());
}

void URVComboComponent::OnComboMontageBlendingOut(UAnimMontage* /*InMontage*/, bool /*bInterrupted*/)
{
    EndCombo();
}