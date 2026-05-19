#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVEquipmentComponent.generated.h"

class URVWeaponDataAsset;
class URVHitReactionAnimDataAsset;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRVOnWeaponChanged, URVWeaponDataAsset*, NewWeaponData);

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URVEquipmentComponent();

	URVWeaponDataAsset*          GetCurrentWeaponData()         const { return CurrentWeaponData; }
	URVHitReactionAnimDataAsset* GetCurrentHitReactionAnimData() const;
	UStaticMeshComponent*        GetWeaponMeshComponent()        const { return WeaponMeshComponent; }

	void SetCurrentWeaponData(URVWeaponDataAsset* InWeaponData);

	// Toggles between WeaponDataSlotA and WeaponDataSlotB.
	// Gate checks (combat state) are the caller's responsibility before calling this.
	void SwapWeapon();

	UPROPERTY(BlueprintAssignable, Category = "RV|Equipment")
	FRVOnWeaponChanged OnWeaponChanged;

	// Slot A is the default starting weapon (equipped in BeginPlay).
	UPROPERTY(EditDefaultsOnly, Category = "RV|Equipment")
	TObjectPtr<URVWeaponDataAsset> WeaponDataSlotA;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Equipment")
	TObjectPtr<URVWeaponDataAsset> WeaponDataSlotB;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "RV|Equipment")
	TObjectPtr<URVWeaponDataAsset> CurrentWeaponData;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> WeaponMeshComponent;

	bool bIsSlotA = true;
};
