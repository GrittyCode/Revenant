// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVAttributeComponent.generated.h"


class URVCharacterDataAsset;

// NewHealth: value after change / InDelta: positive = heal, negative = damage
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRVOnHealthChanged, float, NewHealth, float, InDelta);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRVOnStaminaChanged, float, NewStamina, float, InDelta);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRVOnDeath);

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	URVAttributeComponent();

	// Called by ARVCharacterBase::BeginPlay after DatAsset is Validated
	void InitializeFromData(const URVCharacterDataAsset* InData);


	// =================== Action Section =============================
public:
	// Returns true if the hit was lethal (triggers OnDeath BroadCast)
	bool ApplyDamage(float InDamageAmount, AActor* InInstigator);

	bool ApplyHealing(float InHealAmount);

	// Returns false if stamina is insufficient - caller decides whether to block the action
	bool ApplyStaminaCost(float InAmount);


	// =================== Outward delegates (mirrors GAS AttributeChangedDelegate pattern) =============================
	UPROPERTY(BlueprintAssignable, Category = "RV|Attribute") //Dynamic MultiCast Delegate -> BlueprintAssignable property
	FRVOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "RV|Attribute")
	FRVOnStaminaChanged OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "RV|Attribute")
	FRVOnDeath OnDeath;


	// =================== Getter Section ===========================
public:
	UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
	float GetStaminaPercent() const;

	UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
	float GetCurrentStamina() const { return CurrentStamina; }

	UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
	float GetMaxStamina() const { return MaxStamina; }

private:
	UPROPERTY(VisibleAnywhere, Category = "RV|Attribute")
	float CurrentHealth = 0.f;

	UPROPERTY(VisibleAnywhere, Category = "RV|Attribute")
	float MaxHealth = 100.f;

	UPROPERTY(VisibleAnywhere, Category = "RV|Attribute")
	float CurrentStamina = 0.f;

	UPROPERTY(VisibleAnywhere, Category = "RV|Attribute")
	float MaxStamina = 100.f;

	// Cached for W2 stamina regen timer
	UPROPERTY(VisibleAnywhere, Category = "RV|Attribute")
	float StaminaRegenRate = 20.f;

	bool bIsDead = false;
};
