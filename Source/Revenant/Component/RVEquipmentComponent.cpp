#include "Component/RVEquipmentComponent.h"
#include "Data/RVWeaponDataAsset.h"
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
	ensureMsgf(IsValid(OwnerCharacter), TEXT("[%s] EquipmentComponent owner must be ACharacter"), *GetNameSafe(GetOwner()));

	WeaponMeshComponent = NewObject<UStaticMeshComponent>(GetOwner(), TEXT("WeaponMeshComponent"));
	WeaponMeshComponent->RegisterComponent();
	WeaponMeshComponent->AttachToComponent(
		OwnerCharacter->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		FName("weapon_r")
	);
	
	SetCurrentWeaponData(DefaultWeaponData);
}

void URVEquipmentComponent::SetCurrentWeaponData(URVWeaponDataAsset* InWeaponData)
{
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