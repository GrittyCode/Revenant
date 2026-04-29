// Source/Revenant/Component/RVEquipmentComponent.cpp
#include "Component/RVEquipmentComponent.h"

URVEquipmentComponent::URVEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URVEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	SetCurrentWeaponData(DefaultWeaponData);
}

void URVEquipmentComponent::SetCurrentWeaponData(URVWeaponDataAsset* InWeaponData)
{
	CurrentWeaponData = InWeaponData;
	OnWeaponChanged.Broadcast(CurrentWeaponData);
}