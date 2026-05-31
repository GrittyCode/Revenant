#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVEquipmentComponent.generated.h"

class URVWeaponDataAsset;
class URVHitReactionAnimDataAsset;
class UStaticMeshComponent;
class UNiagaraComponent;
class UNiagaraSystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRVOnWeaponChanged, URVWeaponDataAsset*, NewWeaponData);

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URVEquipmentComponent();
	virtual void BeginPlay() override;

	URVWeaponDataAsset*          GetCurrentWeaponData()          const { return CurrentWeaponData; }
	URVHitReactionAnimDataAsset* GetCurrentHitReactionAnimData() const;
	UStaticMeshComponent*        GetWeaponMeshComponent()        const { return WeaponMeshComponent; }

	void SetCurrentWeaponData(URVWeaponDataAsset* InWeaponData);

	// Toggles between WeaponDataSlotA and WeaponDataSlotB.
	void SwapWeapon();

	// Called by AnimNotifyState_WeaponTrailFX::NotifyBegin.
	void ActivateWeaponTrail();

	// Called by AnimNotifyState_WeaponTrailFX::NotifyEnd.
	void DeactivateWeaponTrail();

	UPROPERTY(BlueprintAssignable, Category = "RV|Equipment")
	FRVOnWeaponChanged OnWeaponChanged;

	// Slot A is the default starting weapon (equipped in BeginPlay).
	UPROPERTY(EditDefaultsOnly, Category = "RV|Equipment")
	TObjectPtr<URVWeaponDataAsset> WeaponDataSlotA;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Equipment")
	TObjectPtr<URVWeaponDataAsset> WeaponDataSlotB;

private:
	UPROPERTY(VisibleAnywhere, Category = "RV|Equipment")
	TObjectPtr<URVWeaponDataAsset> CurrentWeaponData;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> WeaponMeshComponent;

	// Permanently attached Niagara component for the blade trail.
	// Inactive by default; activated only during AttackHitCheck window.
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> WeaponTrailNC;

	bool bIsSlotA = true;

	// Applies the trail asset and width from the given weapon data.
	// Called on weapon equip / swap. Deactivates any active trail first.
	void SetupWeaponTrail(URVWeaponDataAsset* InWeaponData);
};