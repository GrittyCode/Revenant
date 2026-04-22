// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/RVAttributeComponent.h"
#include "Data/RVCharacterDataAsset.h"

// Sets default values for this component's properties
URVAttributeComponent::URVAttributeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

void URVAttributeComponent::InitializeFromData(const URVCharacterDataAsset* InData)
{
	if (!IsValid(InData))
	{
		UE_LOG(LogTemp, Warning, TEXT("[URVAttributeComponent] InitializeFromData: InData is null on %s"),
		       *GetOwner()->GetName());
		return;
	}

	MaxHealth = InData->MaxHealth;
	CurrentHealth = MaxHealth;

	MaxStamina = InData->MaxStamina;
	CurrentStamina = MaxStamina;

	StaminaRegenRate = InData->StaminaRegenRate;

	bIsDead = false;
}

bool URVAttributeComponent::ApplyDamage(float InDamageAmount, AActor* InInstigator)
{
	if (bIsDead || InDamageAmount <= 0.f) { return false; }

	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - InDamageAmount, 0.f, MaxHealth);
	const float Delta = CurrentHealth - PreviousHealth;

	OnHealthChanged.Broadcast(CurrentHealth, Delta);

	if (CurrentHealth <= 0.f)
	{
		bIsDead = true;
		OnDeath.Broadcast();
		return true;
	}

	return false;
}

bool URVAttributeComponent::ApplyHealing(float InHealAmount)
{
	if (bIsDead || InHealAmount <= 0.f || CurrentHealth >= MaxHealth) { return false; }

	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth + InHealAmount, 0.f, MaxHealth);
	const float Delta = CurrentHealth - PreviousHealth;

	OnHealthChanged.Broadcast(CurrentHealth, Delta);
	return true;
}

bool URVAttributeComponent::ConsumeStamina(float InAmount)
{
	if (InAmount <= 0.f) { return true; } 
	if (CurrentStamina < InAmount) { return false; } 

	CurrentStamina -= InAmount;
	OnStaminaChanged.Broadcast(CurrentStamina, -InAmount);
	return true;
}


float URVAttributeComponent::GetHealthPercent() const
{
	if (MaxHealth <= 0.f) { return 0.f; }
	return CurrentHealth / MaxHealth;
}

float URVAttributeComponent::GetStaminaPercent() const
{
	if (MaxStamina <= 0.f) { return 0.f; }
	return CurrentStamina / MaxStamina;
}

bool URVAttributeComponent::IsAlive() const
{
	return !bIsDead;
}
