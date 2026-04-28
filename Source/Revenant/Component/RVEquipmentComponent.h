// Source/Revenant/Component/RVEquipmentComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVEquipmentComponent.generated.h"

class URVWeaponDataAsset;

UENUM(BlueprintType)
enum class ERVActionType : uint8
{
	TypeA,  // 1H-A style
	TypeB   // 2H-A style
};

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URVEquipmentComponent();

	/** Returns the active weapon DataAsset. May be null if not assigned. */
	URVWeaponDataAsset* GetCurrentWeaponData() const { return CurrentWeaponData; }

	ERVActionType GetActionType() const { return ActionType; }
	void SetActionType(ERVActionType InNewType);

	UPROPERTY(EditDefaultsOnly, Category = "RV|Equipment")
	TObjectPtr<URVWeaponDataAsset> DefaultWeaponData;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "RV|Equipment")
	TObjectPtr<URVWeaponDataAsset> CurrentWeaponData;

	UPROPERTY(VisibleAnywhere, Category = "RV|Equipment")
	ERVActionType ActionType = ERVActionType::TypeA;
};