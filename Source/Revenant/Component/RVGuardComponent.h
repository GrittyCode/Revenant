#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "RVGuardComponent.generated.h"

class ACharacter;
class URVAttributeComponent;
class URVEquipmentComponent;
class URVCombatStateComponent;

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
	 */
	UFUNCTION()
	void OnStaminaDepletedHandler();

	void InitReferences(ACharacter* InOwnerCharacter,
						URVCombatStateComponent* InCombatStateComponent,
						URVAttributeComponent* InAttributeComponent,
						URVEquipmentComponent* InEquipmentComponent);

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

	UPROPERTY(EditDefaultsOnly, Category = "RV|Guard")
	float GuardBreakRecoveryTime = 2.f;

	FTimerHandle GuardBreakRecoveryHandle;

	UFUNCTION()
	void OnGuardBreakRecoveryComplete();
};