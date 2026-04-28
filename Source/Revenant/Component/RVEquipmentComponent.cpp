// Source/Revenant/Component/RVEquipmentComponent.cpp
#include "Component/RVEquipmentComponent.h"

URVEquipmentComponent::URVEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URVEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentWeaponData = DefaultWeaponData;
}

void URVEquipmentComponent::SetActionType(ERVActionType InNewType)
{
	ActionType = InNewType;
}