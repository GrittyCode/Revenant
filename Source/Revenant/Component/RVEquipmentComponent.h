#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVEquipmentComponent.generated.h"

class URVWeaponDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRVOnWeaponChanged, URVWeaponDataAsset*, NewWeaponData);

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URVEquipmentComponent();

	/** Returns the active weapon DataAsset. May be null if not assigned. */
	URVWeaponDataAsset* GetCurrentWeaponData() const { return CurrentWeaponData; }

	/**
	 * Swaps the active weapon DataAsset and broadcasts OnWeaponChanged.
	 * Called from BeginPlay (default weapon) and Phase 2 A/B toggle.
	 */
	void SetCurrentWeaponData(URVWeaponDataAsset* InWeaponData);

	// Fired whenever CurrentWeaponData changes — subscribers update locomotion BS, montages, etc.
	UPROPERTY(BlueprintAssignable, Category = "RV|Equipment")
	FRVOnWeaponChanged OnWeaponChanged;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Equipment")
	TObjectPtr<URVWeaponDataAsset> DefaultWeaponData;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "RV|Equipment")
	TObjectPtr<URVWeaponDataAsset> CurrentWeaponData;
};