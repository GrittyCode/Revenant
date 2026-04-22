// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Base/RVCharacterBase.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVComboComponent.h"
#include "Data/RVCharacterDataAsset.h"

DEFINE_LOG_CATEGORY(LogRVCharacterBase);

// Sets default values
ARVCharacterBase::ARVCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	InitializeComponents();
}

// Called when the game starts or when spawned
void ARVCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (!IsValid(CharacterData) && IsValid(CharacterData->DefaultWeaponData))
	{
		UE_LOG(LogRVCharacterBase, Warning,
			TEXT("[RVCharacterBase: CharacterData not assigned in Blueprint defaults."));
		return;
	}

	AttributeComponent->InitializeFromData(CharacterData);
}

void ARVCharacterBase::InitializeComponents()
{
	AttributeComponent = CreateDefaultSubobject<URVAttributeComponent>(TEXT("AttributeComponent"));
	ComboComponent = CreateDefaultSubobject<URVComboComponent>(TEXT("ComboComponent"));
}


