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

void URVComboComponent::TryAdvanceCombo()
{
    if (!bIsComboActive) { return; }

    if (bHasComboInput)
    {
        // Consume stamina for the next hit before advancing.
        // If stamina is insufficient the combo ends naturally here —
        // the player cannot continue the chain without resources.
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
    else
    {
        // Window opened but no input — combo ends after this section
        EndCombo();
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
	bComboWindowOpen = false;
    ComboCount     = 0;

	// Notify CombatComponent: clears bIsAttacking
    AttributeComponent->ResumeStaminaRegen();
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