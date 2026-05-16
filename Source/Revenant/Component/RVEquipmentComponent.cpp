// Source/Revenant/Component/RVEquipmentComponent.cpp
#include "Component/RVEquipmentComponent.h"
#include "Data/RVWeaponDataAsset.h"
#include "Data/RVLocomotionAnimDataAsset.h"
#include "Data/RVCombatAnimDataAsset.h"
#include "Data/RVCombatDataAsset.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"

URVEquipmentComponent::URVEquipmentComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URVEquipmentComponent::BeginPlay()
{
    Super::BeginPlay();

    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    ensureMsgf(IsValid(OwnerCharacter),
        TEXT("[%s] EquipmentComponent owner must be ACharacter"), *GetNameSafe(GetOwner()));

    WeaponMeshComponent = NewObject<UStaticMeshComponent>(GetOwner(), TEXT("WeaponMeshComponent"));
    WeaponMeshComponent->RegisterComponent();
    WeaponMeshComponent->AttachToComponent(
        OwnerCharacter->GetMesh(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        FName("weapon_r")
    );

    SetCurrentWeaponData(DefaultWeaponData);
}

URVCombatDataAsset* URVEquipmentComponent::GetCurrentCombatData() const
{
    return IsValid(CurrentWeaponData) ? CurrentWeaponData->CombatData : nullptr;
}

void URVEquipmentComponent::SetCurrentWeaponData(URVWeaponDataAsset* InWeaponData)
{
    ensureMsgf(!IsValid(InWeaponData) || IsValid(InWeaponData->LocomotionAnimData),
        TEXT("[%s] WeaponDataAsset '%s' has no LocomotionAnimData assigned"),
        *GetNameSafe(GetOwner()), *GetNameSafe(InWeaponData));

    ensureMsgf(!IsValid(InWeaponData) || IsValid(InWeaponData->CombatAnimData),
        TEXT("[%s] WeaponDataAsset '%s' has no CombatAnimData assigned"),
        *GetNameSafe(GetOwner()), *GetNameSafe(InWeaponData));

    ensureMsgf(!IsValid(InWeaponData) || IsValid(InWeaponData->CombatData),
        TEXT("[%s] WeaponDataAsset '%s' has no CombatData assigned"),
        *GetNameSafe(GetOwner()), *GetNameSafe(InWeaponData));

    CurrentWeaponData = InWeaponData;

    if (IsValid(WeaponMeshComponent))
    {
        UStaticMesh* NewMesh = nullptr;
        if (IsValid(InWeaponData) && !InWeaponData->WeaponMesh.IsNull())
        {
            NewMesh = InWeaponData->WeaponMesh.LoadSynchronous();
        }
        WeaponMeshComponent->SetStaticMesh(NewMesh);

        if (IsValid(InWeaponData))
        {
            WeaponMeshComponent->SetRelativeTransform(InWeaponData->WeaponAttachTransform);
        }
    }

    OnWeaponChanged.Broadcast(CurrentWeaponData);
}