#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "RVGuardComponent.generated.h"

class ACharacter;
class URVAttributeComponent;
class URVEquipmentComponent;
class URVCombatStateComponent;

/**
 * Fires when guard break is triggered.
 * Carries the GuardBreakMontage resolved from the current weapon.
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FRVOnGuardBreakTriggered, UAnimMontage* /*GuardBreakMontage*/);

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVGuardComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVGuardComponent();

    void StartGuard();
    void EndGuard();
    void HandleGuardHit(float InDamageAmount);

    /** Called via CombatStateComponent::OnForceEnd. */
    void ForceEndGuard();

    /**
     * Subscribed to URVAttributeComponent::OnStaminaDepleted.
     * Interprets depletion as guard break only when Guarding state is active.
     * Broadcasts OnGuardBreakTriggered so HitReactionComponent handles recovery
     * through the HitReaction path — no separate GuardBroken state needed.
     */
    UFUNCTION()
    void OnStaminaDepletedHandler();

    void InitReferences(ACharacter* InOwnerCharacter,
                        URVCombatStateComponent* InCombatStateComponent,
                        URVAttributeComponent* InAttributeComponent,
                        URVEquipmentComponent* InEquipmentComponent);

    /**
     * Fired when guard break triggers.
     * Wired by ARVCharacterBase::BeginPlay:
     *   GuardComponent->OnGuardBreakTriggered.AddUObject(
     *       HitReactionComponent, &URVHitReactionComponent::TriggerStaggerWithMontage)
     */
    FRVOnGuardBreakTriggered OnGuardBreakTriggered;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<URVCombatStateComponent> CombatStateComponent;

    UPROPERTY()
    TObjectPtr<URVAttributeComponent> AttributeComponent;

    UPROPERTY()
    TObjectPtr<URVEquipmentComponent> EquipmentComponent;
};