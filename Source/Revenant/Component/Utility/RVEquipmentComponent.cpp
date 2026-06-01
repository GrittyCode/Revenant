#include "Component/Utility/RVEquipmentComponent.h"
#include "Data/Asset/RVWeaponDataAsset.h"
#include "Data/Asset/RVLocomotionAnimDataAsset.h"
#include "Data/Asset/RVPlayerCombatAnimDataAsset.h"
#include "Data/Asset/RVHitReactionAnimDataAsset.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

static const FName ParamTrailWidth = TEXT("Width");
static const FName SocketWeaponMiddle = TEXT("WeaponMiddle");

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
    WeaponTrailNC = NewObject<UNiagaraComponent>(GetOwner(), TEXT("WeaponTrailNC"));
    WeaponTrailNC->bAutoActivate = false;
    WeaponTrailNC->RegisterComponent();
    WeaponTrailNC->AttachToComponent(
        WeaponMeshComponent,
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        SocketWeaponMiddle);

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

    SetupWeaponTrail(InWeaponData);

    OnWeaponChanged.Broadcast(CurrentWeaponData);
}

// ----------------------------------------------------------------------------

void URVEquipmentComponent::SetupWeaponTrail(URVWeaponDataAsset* InWeaponData)
{
    DeactivateWeaponTrail(); // ensure trail is off before swapping asset

    UNiagaraSystem* NewAsset = IsValid(InWeaponData) ? InWeaponData->TrailEffect : nullptr;
    WeaponTrailNC->SetAsset(NewAsset);

    if (IsValid(NewAsset) && IsValid(InWeaponData))
    {
        WeaponTrailNC->SetVariableFloat(ParamTrailWidth, InWeaponData->TrailWidth);
    }
}

void URVEquipmentComponent::ActivateWeaponTrail()
{
    if (!IsValid(WeaponTrailNC->GetAsset())) { return; }
    WeaponTrailNC->Activate(true);
}

void URVEquipmentComponent::DeactivateWeaponTrail()
{
    if (WeaponTrailNC->IsActive()) { WeaponTrailNC->DeactivateImmediate(); }
}