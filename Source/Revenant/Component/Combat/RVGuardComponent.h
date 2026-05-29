#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVGuardComponent.generated.h"

class ARVCharacterBase;
class ARVCharacterPlayer;
class URVStaminaComponent;
class URVEquipmentComponent;

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVGuardComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVGuardComponent();

    void StartGuard();
    void EndGuard();
    void HandleGuardHit(float InDamageAmount);

    UFUNCTION()
    void OnStaminaDepletedHandler();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TObjectPtr<ARVCharacterBase> OwnerBase;

    // Cached from OwnerBase — set once in BeginPlay after player cast is verified.
    UPROPERTY()
    TObjectPtr<URVStaminaComponent> StaminaComponent;

    UPROPERTY()
    TObjectPtr<URVEquipmentComponent> EquipmentComponent;
};
