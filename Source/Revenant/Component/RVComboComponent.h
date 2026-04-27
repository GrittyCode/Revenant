// Source/Revenant/Component/RVComboComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVComboComponent.generated.h"

class URVAttributeComponent;
class URVEquipmentComponent;
class URVWeaponDataAsset;

DECLARE_LOG_CATEGORY_EXTERN(LogRVCombo, Log, All);

/**
 * Manages combo state, input buffering, and montage section sequencing.
 *
 * Single responsibility: combo logic only.
 * Weapon data is read from URVEquipmentComponent — this component does not own it.
 * Stamina cost is applied via URVAttributeComponent — cached in BeginPlay.
 */
UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVComboComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URVComboComponent();

	/**
	 * Combo entry point.
	 * - Not active  → StartCombo()
	 * - Active      → buffer input (bComboInputPending)
	 * Callers: ARVCharacterPlayer (Enhanced Input), BTTask_MeleeAttack (enemy/boss)
	 */
	void HandleComboInput();

	/**
	 * Called by AnimNotify_ComboWindow at the branch point of each montage section.
	 * Advances to the next section if input is buffered; otherwise lets montage end naturally.
	 */
	void TryAdvanceCombo();

	UFUNCTION(BlueprintCallable, Category = "RV|Combo")
	bool IsComboActive() const { return bComboActive; }

	UFUNCTION(BlueprintCallable, Category = "RV|Combo")
	int32 GetCurrentComboCount() const { return CurrentComboCount; }

protected:
	virtual void BeginPlay() override;

private:
	/** Convenience accessor — queries EquipmentComponent. Returns nullptr if not equipped. */
	URVWeaponDataAsset* GetWeaponData() const;

	void StartCombo();
	void AdvanceToNextCombo();
	void ResetCombo();

	// Bound once in BeginPlay — filtered by montage pointer inside the callback
	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* InMontage, bool bInInterrupted);

	UAnimInstance* GetOwnerAnimInstance() const;

	// Cached in BeginPlay — sibling components on the same owner
	UPROPERTY()
	TObjectPtr<URVEquipmentComponent> EquipmentComponent;

	UPROPERTY()
	TObjectPtr<URVAttributeComponent> AttributeComponent;

	bool bComboActive       = false;
	bool bComboInputPending = false;

	int32 CurrentComboCount = 0;
};