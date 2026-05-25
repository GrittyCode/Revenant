#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "RVDodgeComponent.generated.h"

class ACharacter;
class URVAttributeComponent;
class URVCombatStateComponent;
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

	void ForceEndDodge();

	void InitReferences(ACharacter* InOwnerCharacter,
						URVCombatStateComponent* InCombatStateComponent,
						URVAttributeComponent* InAttributeComponent,
						float InDodgeStaminaCost);

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

	float DodgeStaminaCost = 30.f;

	// Guards OnDodgeMontageBlendingOut against stale callbacks on external interruption.
	UPROPERTY()
	TObjectPtr<UAnimMontage> ActiveDodgeMontage;

	void EndDodge();
	void OnDodgeMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);
};