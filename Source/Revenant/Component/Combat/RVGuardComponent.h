#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVGuardComponent.generated.h"

class ARVCharacterBase;
class URVStaminaComponent;
class URVEquipmentComponent;

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVGuardComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URVGuardComponent();
	virtual void BeginPlay() override;

	// Called from ARVCharacterPlayer::BeginPlay after all components exist.
	void Init(URVStaminaComponent* InStamina, URVEquipmentComponent* InEquipment);

	void StartGuard();
	void EndGuard();
	void HandleGuardHit(float InDamageAmount);

	UFUNCTION()
	void OnStaminaDepletedHandler();

private:
	UPROPERTY()
	TObjectPtr<ARVCharacterBase> OwnerBase;

	UPROPERTY()
	TObjectPtr<URVStaminaComponent> StaminaComponent;

	UPROPERTY()
	TObjectPtr<URVEquipmentComponent> EquipmentComponent;
};