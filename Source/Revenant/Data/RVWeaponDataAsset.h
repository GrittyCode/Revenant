#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Data/RVPlayerCombatAnimDataAsset.h"
#include "RVWeaponDataAsset.generated.h"

class URVLocomotionAnimDataAsset;
class URVPlayerCombatAnimDataAsset;
class URVHitReactionAnimDataAsset;
class UNiagaraSystem;
class USoundBase;
class UTexture2D;
struct FRVWeaponStatRow;

UCLASS(BlueprintType)
class REVENANT_API URVWeaponDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:

	//--- Stat ----------------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat")
	FDataTableRowHandle WeaponStatRowHandle;

	//--- Weapon Mesh ---------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	TSoftObjectPtr<UStaticMesh> WeaponMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Transform")
	FTransform WeaponAttachTransform;

	//--- Animation -----------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<URVLocomotionAnimDataAsset> LocomotionAnimData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<URVPlayerCombatAnimDataAsset> CombatAnimData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<URVHitReactionAnimDataAsset> HitReactionAnimData;
	
	
	//--- VFX / SFX -----------------------------------------------------------

	// Weapon trail Niagara system.
	// Activated during AttackHitCheck window via AnimNotifyState_WeaponTrailFX.
	// Must expose User Parameters: TrailWorldStart (Vector), TrailWorldEnd (Vector), TrailWidth (Float).
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<UNiagaraSystem> TrailEffect;

	// Ribbon width of the blade trail. Injected into TrailWidth Niagara parameter.
	UPROPERTY(EditDefaultsOnly, Category = "VFX", meta = (ClampMin = "0.0"))
	float TrailWidth = 10.f;

	// Niagara hit impact spawned at the struck actor's location on each confirmed hit.
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<UNiagaraSystem> HitImpactEffect;

	// Sound played at the struck actor's location on each confirmed hit.
	UPROPERTY(EditDefaultsOnly, Category = "SFX")
	TObjectPtr<USoundBase> HitSFX;

	//--- Per-Instance Montage Overrides --------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Override|HeavyAttack",
	          meta = (InlineEditConditionToggle))
	uint8 bOverrideHeavyChargeMontage : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Override|HeavyAttack",
	          meta = (EditCondition = "bOverrideHeavyChargeMontage"))
	TObjectPtr<UAnimMontage> OverrideHeavyChargeMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Override|HeavyAttack",
	          meta = (InlineEditConditionToggle))
	uint8 bOverrideHeavyAttackMontage : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Override|HeavyAttack",
	          meta = (EditCondition = "bOverrideHeavyAttackMontage"))
	TObjectPtr<UAnimMontage> OverrideHeavyAttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Override|HeavyAttack",
	          meta = (EditCondition = "bOverrideHeavyAttackMontage"))
	TObjectPtr<UAnimMontage> OverrideMaxHeavyAttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Override|Dodge",
	          meta = (InlineEditConditionToggle))
	uint8 bOverrideDodgeMontage : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Override|Dodge",
	          meta = (EditCondition = "bOverrideDodgeMontage"))
	TObjectPtr<UAnimMontage> OverrideDodgeMontage;

	//--- Getters -------------------------------------------------------------

	const FRVWeaponStatRow* GetWeaponStatRow() const;

	UAnimMontage* GetHeavyChargeMontage() const;

	UAnimMontage* GetHeavyAttackMontage(bool bIsMax) const;

	UAnimMontage* GetDodgeMontage() const;

	// Returns the lock-on dodge montage for the given direction.
	UAnimMontage* GetLockOnDodgeMontage(ERVDodgeDirection InDirection) const;
};