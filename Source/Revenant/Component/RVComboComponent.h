#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVComboComponent.generated.h"

class ACharacter;
class URVCombatStateComponent;
class URVAttributeComponent;
class URVEquipmentComponent;
class URVWeaponDataAsset;
class UAnimMontage;

DECLARE_MULTICAST_DELEGATE(FRVOnComboStarted);
DECLARE_MULTICAST_DELEGATE(FRVOnComboEnded);

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVComboComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URVComboComponent();

	void HandleComboInput();
	void TryChainNextCombo();

	void OpenComboWindow();
	void CloseComboWindow();

	void ForceEndCombo();

	bool IsComboActive() const { return bIsComboActive; }

	FRVOnComboStarted OnComboStarted;
	FRVOnComboEnded   OnComboEnded;

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

	bool bIsComboActive   = false;
	bool bComboWindowOpen = false;
	bool bHasComboInput   = false;

	void StartCombo();
	void EndCombo();
	void PlayComboMontage(UAnimMontage* InMontage);

	// Returns false if stamina was insufficient (caller should abort the combo).
	bool ConsumeComboStamina(UAnimMontage* InMontage, const URVWeaponDataAsset* InWeaponData);

	UFUNCTION()
	void OnComboMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
};