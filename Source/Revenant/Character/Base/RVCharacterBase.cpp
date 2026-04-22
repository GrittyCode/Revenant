// Source/Revenant/Character/Base/RVCharacterBase.cpp
#include "Character/Base/RVCharacterBase.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVComboComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Data/RVCharacterDataAsset.h"

DEFINE_LOG_CATEGORY(LogRVCharacterBase);

ARVCharacterBase::ARVCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	InitializeComponents();
}

void ARVCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (!IsValid(CharacterData))
	{
		UE_LOG(LogRVCharacterBase, Warning,
			TEXT("[%s] BeginPlay: CharacterData not assigned in Blueprint defaults."), *GetName());
		return;
	}

	AttributeComponent->InitializeFromData(CharacterData);

	if (IsValid(CharacterData->DefaultWeaponData))
	{
		EquipmentComponent->EquipWeapon(CharacterData->DefaultWeaponData);
	}
}

void ARVCharacterBase::InitializeComponents()
{
	AttributeComponent  = CreateDefaultSubobject<URVAttributeComponent>(TEXT("AttributeComponent"));
	ComboComponent      = CreateDefaultSubobject<URVComboComponent>(TEXT("ComboComponent"));
	EquipmentComponent  = CreateDefaultSubobject<URVEquipmentComponent>(TEXT("EquipmentComponent"));
}