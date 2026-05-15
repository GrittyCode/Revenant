#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVEquipmentComponent.generated.h"

class URVWeaponDataAsset;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRVOnWeaponChanged, URVWeaponDataAsset*, NewWeaponData);

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URVEquipmentComponent();

	URVWeaponDataAsset* GetCurrentWeaponData() const { return CurrentWeaponData; }

	/** Returns the runtime weapon static mesh component. Null before BeginPlay. */
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

	// Runtime static mesh component for the equipped weapon.
	// Created in BeginPlay, attached to the owner character's weapon_r socket.
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> WeaponMeshComponent;
};