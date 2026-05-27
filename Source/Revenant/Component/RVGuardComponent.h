#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVGuardComponent.generated.h"

class ARVCharacterBase;
class IRVWeaponUser;

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVGuardComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVGuardComponent();

    void StartGuard();
    void EndGuard();
    void HandleGuardHit(float InDamageAmount);

    // Subscribed to AttributeComponent::OnStaminaDepleted by ARVCharacterPlayer::BeginPlay.
    UFUNCTION()
    void OnStaminaDepletedHandler();

protected:
    virtual void BeginPlay() override;

private:
    // Resolved in BeginPlay via GetOwner().
    UPROPERTY()
    TObjectPtr<ARVCharacterBase> OwnerBase;

    IRVWeaponUser* WeaponUser = nullptr;
};
