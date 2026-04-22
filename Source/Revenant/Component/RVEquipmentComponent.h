// Source/Revenant/Component/RVEquipmentComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVEquipmentComponent.generated.h"

class URVWeaponDataAsset;

DECLARE_LOG_CATEGORY_EXTERN(LogRVEquipment, Log, All);


UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URVEquipmentComponent();

	/** Returns the currently equipped weapon data. Nullptr if nothing is equipped. */
	UFUNCTION(BlueprintCallable, Category = "RV|Equipment")
	URVWeaponDataAsset* GetWeaponData() const { return WeaponData; }

	/**
	 * Swap the equipped weapon.
	 * Caller is responsible for timing (do not call while a combo is active).
	 */
	UFUNCTION(BlueprintCallable, Category = "RV|Equipment")
	void EquipWeapon(URVWeaponDataAsset* InWeaponData);

private:
	// Assigned via EquipWeapon() — first call is from ARVCharacterBase::BeginPlay
	// using CharacterData->DefaultWeaponData.
	UPROPERTY(VisibleAnywhere, Category = "RV|Equipment", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URVWeaponDataAsset> WeaponData;
};