// Source/Revenant/Component/RVEquipmentComponent.cpp
#include "Component/RVEquipmentComponent.h"
#include "Data/RVWeaponDataAsset.h"

DEFINE_LOG_CATEGORY(LogRVEquipment);

URVEquipmentComponent::URVEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URVEquipmentComponent::EquipWeapon(URVWeaponDataAsset* InWeaponData)
{
	WeaponData = InWeaponData;

	UE_LOG(LogRVEquipment, Log, TEXT("[%s] EquipWeapon: %s"),
		*GetOwner()->GetName(),
		IsValid(InWeaponData) ? *InWeaponData->GetName() : TEXT("None"));
}