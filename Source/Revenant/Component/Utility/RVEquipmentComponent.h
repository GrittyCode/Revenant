#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVEquipmentComponent.generated.h"

class URVWeaponDataAsset;
class URVHitReactionAnimDataAsset;
class UStaticMeshComponent;
class UNiagaraComponent;
class UNiagaraSystem;

DECLARE_MULTICAST_DELEGATE_OneParam(FRVOnWeaponChanged, URVWeaponDataAsset*);

UCLASS(ClassGroup=(Revenant))
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
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> WeaponTrailNC;

	bool bIsSlotA = true;

	// Applies the trail asset and width from the given weapon data.
	void SetupWeaponTrail(URVWeaponDataAsset* InWeaponData);
};