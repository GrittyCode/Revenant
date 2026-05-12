#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "RVDodgeComponent.generated.h"

class ACharacter;
class URVAttributeComponent;
class URVEquipmentComponent;
class URVCombatStateComponent;
class URVCharacterDataAsset;
class UAnimMontage;

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVDodgeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URVDodgeComponent();

	void StartDodge(const FVector& InDodgeDirection);

	/**
	 * Called by AnimNotifyState_DodgeIFrame.
	 * Guards against activation if dodge was externally interrupted.
	 */
	void SetDodgeIFrame(bool bActivate);

	/** Called via CombatStateComponent::OnForceEnd. */
	void ForceEndDodge();

	void InitReferences(ACharacter* InOwnerCharacter,
						URVCombatStateComponent* InCombatStateComponent,
						URVAttributeComponent* InAttributeComponent,
						URVEquipmentComponent* InEquipmentComponent,
						URVCharacterDataAsset* InCharacterData);

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

	UPROPERTY()
	TObjectPtr<URVCharacterDataAsset> CharacterData;

	void EndDodge();
	void OnDodgeMontageBlendingOut(UAnimMontage*, bool bInterrupted);
};