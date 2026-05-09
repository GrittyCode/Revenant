#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "RVSprintComponent.generated.h"

class URVCombatStateComponent;
class URVAttributeComponent;
class UCharacterMovementComponent;

/**
 * Manages sprint start, sprint speed, and self-termination.
 * Subscribes to URVCombatStateComponent::OnStateChanged — when any blocking
 * combat state becomes active, Sprint ends itself without being told by other components.
 * No action component needs to know about Sprint.
 */
UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVSprintComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVSprintComponent();

    void StartSprint();
    void EndSprint();

    UFUNCTION(BlueprintCallable, Category = "RV|Sprint")
    bool IsSprinting() const { return bIsSprinting; }

    /** Called via CombatStateComponent::OnForceEnd. */
    void ForceEndSprint();

protected:
    virtual void BeginPlay() override;

private:
    // --- Cached References ---------------------------------------------------

    UPROPERTY()
    TObjectPtr<URVCombatStateComponent> CombatStateComponent;

    UPROPERTY()
    TObjectPtr<URVAttributeComponent> AttributeComponent;

    UPROPERTY()
    TObjectPtr<UCharacterMovementComponent> MovementComponent;

    // --- State ---------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "RV|Sprint")
    float SprintSpeed = 1000.f;

    float OriginalWalkSpeed = 0.f;
    bool  bIsSprinting = false;

    // --- Handlers ------------------------------------------------------------

    /**
     * Subscribed to URVCombatStateComponent::OnStateChanged.
     * Ends sprint automatically when any blocking combat state becomes active.
     * No action component needs to call EndSprint explicitly.
     */
    void OnCombatStateChanged(ERVCombatState InNewState);
};