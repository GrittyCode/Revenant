#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "RVDodgeComponent.generated.h"

class ACharacter;
class URVAttributeComponent;
class URVCombatStateComponent;
class URVCharacterDataAsset;
class UAnimMontage;

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVDodgeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URVDodgeComponent();

	// Montage is selected and pre-rotated by the caller (ARVCharacterPlayer::InputDodge).
	// Guard interruption is also handled by the caller before this is invoked.
	void StartDodge(UAnimMontage* InMontage);

	void SetDodgeIFrame(bool bActivate);

	/** Called via CombatStateComponent::OnForceEnd. */
	void ForceEndDodge();

	void InitReferences(ACharacter* InOwnerCharacter,
						URVCombatStateComponent* InCombatStateComponent,
						URVAttributeComponent* InAttributeComponent,
						URVCharacterDataAsset* InCharacterData);
	
	
	bool CanStartDodge() const;


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
	TObjectPtr<URVCharacterDataAsset> CharacterData;

	// Guards OnDodgeMontageBlendingOut against stale callbacks on external interruption.
	UPROPERTY()
	TObjectPtr<UAnimMontage> ActiveDodgeMontage;

	void EndDodge();
	void OnDodgeMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);
};