#include "Component/RVEquipmentComponent.h"
#include "Data/RVWeaponDataAsset.h"
#include "Data/RVLocomotionAnimDataAsset.h"
#include "Data/RVPlayerCombatAnimDataAsset.h"
#include "Data/RVHitReactionAnimDataAsset.h"
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

	// SlotA is the default starting weapon.
	SetCurrentWeaponData(WeaponDataSlotA);
}

URVHitReactionAnimDataAsset* URVEquipmentComponent::GetCurrentHitReactionAnimData() const
{
	return IsValid(CurrentWeaponData) ? CurrentWeaponData->HitReactionAnimData : nullptr;
}

void URVEquipmentComponent::SwapWeapon()
{
	bIsSlotA = !bIsSlotA;
	URVWeaponDataAsset* NextWeapon = bIsSlotA ? WeaponDataSlotA : WeaponDataSlotB;
	if (IsValid(NextWeapon))
	{
		SetCurrentWeaponData(NextWeapon);
	}
}

void URVEquipmentComponent::SetCurrentWeaponData(URVWeaponDataAsset* InWeaponData)
{
	ensureMsgf(!IsValid(InWeaponData) || IsValid(InWeaponData->LocomotionAnimData),
		TEXT("[%s] WeaponDataAsset '%s' has no LocomotionAnimData assigned"),
		*GetNameSafe(GetOwner()), *GetNameSafe(InWeaponData));

	ensureMsgf(!IsValid(InWeaponData) || IsValid(InWeaponData->CombatAnimData),
		TEXT("[%s] WeaponDataAsset '%s' has no CombatAnimData assigned"),
		*GetNameSafe(GetOwner()), *GetNameSafe(InWeaponData));

	ensureMsgf(!IsValid(InWeaponData) || IsValid(InWeaponData->HitReactionAnimData),
		TEXT("[%s] WeaponDataAsset '%s' has no HitReactionAnimData assigned"),
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
