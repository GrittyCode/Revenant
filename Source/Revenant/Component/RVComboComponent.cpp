// Source/Revenant/Component/RVComboComponent.cpp
#include "Component/RVComboComponent.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Data/RVWeaponDataAsset.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"

DEFINE_LOG_CATEGORY(LogRVCombo);

URVComboComponent::URVComboComponent()
	: bComboActive(0)
	  , bComboInputPending(0)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URVComboComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache sibling EquipmentComponent — created in the same InitializeComponents() call
	EquipmentComponent = GetOwner()->FindComponentByClass<URVEquipmentComponent>();
	if (!IsValid(EquipmentComponent))
	{
		UE_LOG(LogRVCombo, Warning, TEXT("[%s] BeginPlay: URVEquipmentComponent not found on owner."),
		       *GetOwner()->GetName());
	}

	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	if (IsValid(AnimInstance))
	{
		AnimInstance->OnMontageEnded.AddDynamic(this, &URVComboComponent::OnAttackMontageEnded);
	}
	else
	{
		UE_LOG(LogRVCombo, Warning, TEXT("[%s] BeginPlay: AnimInstance not found — combo reset will not fire."),
		       *GetOwner()->GetName());
	}
}

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

void URVComboComponent::HandleComboInput()
{
	if (!IsValid(GetWeaponData()))
	{
		return;
	}

	if (!bComboActive)
	{
		StartCombo();
		return;
	}

	// Buffer one input — ignored if already at the last hit
	if (CurrentComboCount < GetWeaponData()->MaxComboCount)
	{
		bComboInputPending = true;
	}
}

void URVComboComponent::TryAdvanceCombo()
{
	if (!bComboActive || !IsValid(GetWeaponData()))
	{
		return;
	}

	if (bComboInputPending && CurrentComboCount < GetWeaponData()->MaxComboCount)
	{
		AdvanceToNextCombo();
	}

	// No pending input → section plays to natural end → OnAttackMontageEnded → ResetCombo
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void URVComboComponent::StartCombo()
{
	URVWeaponDataAsset* CurrentWeaponData = GetWeaponData();
	if (!IsValid(CurrentWeaponData)) { return; }

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!IsValid(OwnerCharacter)) { return; }

	URVAttributeComponent* AttrComp = OwnerCharacter->FindComponentByClass<URVAttributeComponent>();
	if (!IsValid(AttrComp)) { return; }

	// Consume stamina before play — prevents spam during recovery frames
	if (!AttrComp->ApplyStaminaCost(CurrentWeaponData->AttackStaminaCost))
	{
		UE_LOG(LogRVCombo, Log, TEXT("[%s] StartCombo: not enough stamina."), *GetOwner()->GetName());
		return;
	}

	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	if (!IsValid(AnimInstance)) { return; }

	bComboActive = true;
	bComboInputPending = false;
	CurrentComboCount = 1;

	AnimInstance->Montage_Play(CurrentWeaponData->AttackMontage);
	// Montage starts at section index 0 ("Attack1") by default
}

void URVComboComponent::AdvanceToNextCombo()
{
	URVWeaponDataAsset* CurrentWeaponData = GetWeaponData();
	if (!IsValid(CurrentWeaponData)) { return; }

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!IsValid(OwnerCharacter)) { return; }

	URVAttributeComponent* AttrComp = OwnerCharacter->FindComponentByClass<URVAttributeComponent>();
	if (!IsValid(AttrComp)) { return; }

	if (!AttrComp->ApplyStaminaCost(CurrentWeaponData->AttackStaminaCost))
	{
		ResetCombo();
		return;
	}

	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	if (!IsValid(AnimInstance)) { return; }

	bComboInputPending = false;

	// CurrentComboCount is 1-based after StartCombo, so index [CurrentComboCount] = next section
	AnimInstance->Montage_JumpToSection(
		CurrentWeaponData->ComboSectionNames[CurrentComboCount],
		CurrentWeaponData->AttackMontage);

	++CurrentComboCount;
}

void URVComboComponent::ResetCombo()
{
	bComboActive = false;
	bComboInputPending = false;
	CurrentComboCount = 0;
}

void URVComboComponent::OnAttackMontageEnded(UAnimMontage* InMontage, bool bInInterrupted)
{
	const URVWeaponDataAsset* CurrentWeaponData = GetWeaponData();
	if (!IsValid(CurrentWeaponData) || InMontage != CurrentWeaponData->AttackMontage) { return; }

	ResetCombo();
}


URVWeaponDataAsset* URVComboComponent::GetWeaponData() const
{
	if (!IsValid(EquipmentComponent))
	{
		return nullptr;
	}
	return EquipmentComponent->GetWeaponData();
}

UAnimInstance* URVComboComponent::GetOwnerAnimInstance() const
{
	const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());

	if (!IsValid(OwnerCharacter))
	{
		return nullptr;
	}

	return OwnerCharacter->GetMesh()->GetAnimInstance();
}
