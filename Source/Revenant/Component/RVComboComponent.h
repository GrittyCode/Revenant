// Source/Revenant/Component/RVComboComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVComboComponent.generated.h"

class URVEquipmentComponent;
class URVCombatComponent;
class URVAttributeComponent;

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVComboComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URVComboComponent();

	/**
	 * Called by ARVCharacterPlayer on attack input.
	 * Starts the first combo hit or buffers the next hit if already attacking.
	 */
	void HandleComboInput();

	/**
	 * Called by AnimNotify_ComboWindow.
	 * Advances to the next combo section if input was buffered.
	 */
	void TryAdvanceCombo();

	UFUNCTION(BlueprintCallable, Category = "RV|Combo")
	bool IsComboActive() const { return bIsComboActive; }

	UFUNCTION(BlueprintCallable, Category = "RV|Combo")
	int32 GetComboCount() const { return ComboCount; }

protected:
	virtual void BeginPlay() override;

private:
	// --- Cached Component References --------------------------------------------

	UPROPERTY()
	TObjectPtr<URVEquipmentComponent> EquipmentComponent;

	UPROPERTY()
	TObjectPtr<URVCombatComponent> CombatComponent;

	UPROPERTY()
	TObjectPtr<URVAttributeComponent> AttributeComponent;

	// --- State ------------------------------------------------------------------

	bool  bIsComboActive  = false;
	bool  bHasComboInput  = false;
	int32 ComboCount      = 0;

	// --- Internal ---------------------------------------------------------------

	void StartCombo();
	void EndCombo();

	/** Plays the montage section for the current ComboCount. */
	void PlayComboSection();

	/** Bound to montage blend-out -- cleans up when full combo ends or is interrupted. */
	void OnComboMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);
};