#include "Component/RVEquipmentComponent.h"
#include "Data/RVWeaponDataAsset.h"
#include "Data/RVLocomotionAnimDataAsset.h"
#include "Data/RVPlayerCombatAnimDataAsset.h"
#include "Data/RVHitReactionAnimDataAsset.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

// NS_Trail_Sword User parameter name for ribbon width.
static const FName ParamTrailWidth  = TEXT("Width");

// Attachment socket on the weapon StaticMesh — trail ribbon follows this point.
static const FName SocketWeaponTip  = TEXT("WeaponMiddle");

// ----------------------------------------------------------------------------

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

	// ---- Weapon mesh ----
	WeaponMeshComponent = NewObject<UStaticMeshComponent>(GetOwner(), TEXT("WeaponMeshComponent"));
	WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMeshComponent->RegisterComponent();
	WeaponMeshComponent->AttachToComponent(
		OwnerCharacter->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		FName("weapon_r"));

	// ---- Blade trail Niagara component ----
	// Attached to WeaponTip socket — NS_Trail_Sword generates its ribbon from
	// this component's world movement trajectory (velocity-based ribbon, no manual param injection).
	WeaponTrailNC = NewObject<UNiagaraComponent>(GetOwner(), TEXT("WeaponTrailNC"));
	WeaponTrailNC->bAutoActivate = false;
	WeaponTrailNC->RegisterComponent();
	WeaponTrailNC->AttachToComponent(
		WeaponMeshComponent,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		SocketWeaponTip);

	// SlotA is the default starting weapon.
	SetCurrentWeaponData(WeaponDataSlotA);
}

// ----------------------------------------------------------------------------

URVHitReactionAnimDataAsset* URVEquipmentComponent::GetCurrentHitReactionAnimData() const
{
	return IsValid(CurrentWeaponData) ? CurrentWeaponData->HitReactionAnimData : nullptr;
}

void URVEquipmentComponent::SwapWeapon()
{
	bIsSlotA = !bIsSlotA;
	URVWeaponDataAsset* NextWeapon = bIsSlotA ? WeaponDataSlotA : WeaponDataSlotB;
	if (IsValid(NextWeapon)) { SetCurrentWeaponData(NextWeapon); }
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

	SetupWeaponTrail(InWeaponData);

	OnWeaponChanged.Broadcast(CurrentWeaponData);
}

// ----------------------------------------------------------------------------

void URVEquipmentComponent::SetupWeaponTrail(URVWeaponDataAsset* InWeaponData)
{
	if (!IsValid(WeaponTrailNC)) { return; }

	DeactivateWeaponTrail(); // ensure trail is off before swapping asset

	UNiagaraSystem* NewAsset = IsValid(InWeaponData) ? InWeaponData->TrailEffect : nullptr;
	WeaponTrailNC->SetAsset(NewAsset);

	// Set ribbon width once at setup — NS_Trail_Sword reads User.Width parameter.
	if (IsValid(NewAsset) && IsValid(InWeaponData))
	{
		WeaponTrailNC->SetVariableFloat(ParamTrailWidth, InWeaponData->TrailWidth);
	}
}

void URVEquipmentComponent::ActivateWeaponTrail()
{
	if (!IsValid(WeaponTrailNC) || !IsValid(WeaponTrailNC->GetAsset())) { return; }
	WeaponTrailNC->Activate(true);
}

void URVEquipmentComponent::DeactivateWeaponTrail()
{
	if (!IsValid(WeaponTrailNC)) { return; }
	if (WeaponTrailNC->IsActive()) { WeaponTrailNC->DeactivateImmediate(); }
}