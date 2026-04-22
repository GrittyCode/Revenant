// Fill out your copyright notice in the Description page of Project Settings.
#include "Component/RVComboComponent.h"

#include "RVAttributeComponent.h"
#include "Data/RVWeaponDataAsset.h"
#include "GameFramework/Character.h"

DEFINE_LOG_CATEGORY(RVComboComponentLog)

URVComboComponent::URVComboComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URVComboComponent::BeginPlay()
{
	Super::BeginPlay();

	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	if (IsValid(AnimInstance))
	{
		AnimInstance->OnMontageEnded.AddDynamic(this, &URVComboComponent::OnAttackMontageEnded);
	}
	else
	{
		UE_LOG(RVComboComponentLog, Warning, TEXT("[%s] BeginPlay: AnimInstance not found — combo reset will not fire"),
		       *GetOwner()->GetName());
	}
}

void URVComboComponent::TryStartCombo()
{
	if (!IsValid(WeaponData))
	{
		return;
	}

	if (!bComboActive)
	{
		StartFirstCombo();
		return;
	}

	// Buffer one Input - ignored if already at the last hit
	if (CurrentComboCount < WeaponData->MaxComboCount)
	{
		bComboInputPending = true;
	}
}

void URVComboComponent::TryAdvancedCombo()
{
	if (!bComboActive || !IsValid(WeaponData))
	{
		return;
	}

	if (bComboInputPending && CurrentComboCount < WeaponData->MaxComboCount)
	{
		AdvanceToNextCombo();
	}
}

void URVComboComponent::SetWeaponData(URVWeaponDataAsset* InWeaponDataAsset)
{
	if (bComboActive)
	{
		// blocked while combo action 
		return;
	}

	WeaponData = InWeaponDataAsset;
}


void URVComboComponent::StartFirstCombo()
{
	CurrentComboCount = 1;
}

void URVComboComponent::AdvanceToNextCombo()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!IsValid(OwnerCharacter))
	{
		return;
	}

	URVAttributeComponent* AttributeComponent = OwnerCharacter->FindComponentByClass<URVAttributeComponent>();
	if (!IsValid(AttributeComponent))
	{
		return;
	}

	if (!AttributeComponent->ApplyStaminaCost(WeaponData->AttackStaminaCost))
	{
		ResetCombo();
		return;
	}


	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	if (!IsValid(AnimInstance))
	{
		return;
	}

	bComboInputPending = false;

	AnimInstance->Montage_JumpToSection(
		WeaponData->ComboSectionNames[CurrentComboCount],
		WeaponData->AttackMontage);

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
	if (!IsValid(WeaponData) || InMontage != WeaponData->AttackMontage)
	{
		return;
	}
	
	ResetCombo();
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
