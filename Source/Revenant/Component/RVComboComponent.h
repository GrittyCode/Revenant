// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVComboComponent.generated.h"

class URVWeaponDataAsset;

DECLARE_LOG_CATEGORY_EXTERN(RVComboComponentLog, Log, All);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVComboComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	URVComboComponent();

	/**
    * Combo entry point.
    * - Not active  → StartFirstCombo()
    * - Active      → buffer input (bComboInputPending)
    * Callers: ARVCharacterPlayer (Enhanced Input), BTTask_MeleeAttack (enemy/boss)
    */
	void TryStartCombo();
	
	/**
	* Called by AnimNotify_ComboWindow at the branch point of each montage section.
	* Advances to the next section if input is buffered; otherwise lets montage end.
	*/
	void TryAdvancedCombo();
	
	
	UFUNCTION(BlueprintCallable, Category = "RV|Combo")
	void SetWeaponData(URVWeaponDataAsset* InWeaponDataAsset);

	UFUNCTION(BlueprintCallable, Category = "RV|Combo")
	bool IsComboActive() const { return bComboActive; }

	UFUNCTION(BlueprintCallable, Category = "RV|Combo")
	int32 GetCurrentComboCount() const { return CurrentComboCount; }

protected:
	virtual void BeginPlay() override;

private:
	void StartFirstCombo();
	void AdvanceToNextCombo();
	void ResetCombo();

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* InMontage, bool bInInterrupted);

	UAnimInstance* GetOwnerAnimInstance() const;

	UPROPERTY()
	TObjectPtr<URVWeaponDataAsset> WeaponData;

	bool bComboActive = false;
	bool bComboInputPending = false;

	int32 CurrentComboCount = 0;
};
