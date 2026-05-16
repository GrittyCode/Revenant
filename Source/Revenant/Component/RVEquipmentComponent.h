#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVEquipmentComponent.generated.h"

class URVWeaponDataAsset;
class URVCombatDataAsset;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRVOnWeaponChanged, URVWeaponDataAsset*, NewWeaponData);

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URVEquipmentComponent();

	URVWeaponDataAsset* GetCurrentWeaponData() const { return CurrentWeaponData; }
	URVCombatDataAsset* GetCurrentCombatData() const;
	UStaticMeshComponent* GetWeaponMeshComponent() const { return WeaponMeshComponent; }

	void SetCurrentWeaponData(URVWeaponDataAsset* InWeaponData);

	UPROPERTY(BlueprintAssignable, Category = "RV|Equipment")
	FRVOnWeaponChanged OnWeaponChanged;

	UPROPERTY(EditDefaultsOnly, Category = "RV|Equipment")
	TObjectPtr<URVWeaponDataAsset> DefaultWeaponData;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "RV|Equipment")
	TObjectPtr<URVWeaponDataAsset> CurrentWeaponData;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> WeaponMeshComponent;
};