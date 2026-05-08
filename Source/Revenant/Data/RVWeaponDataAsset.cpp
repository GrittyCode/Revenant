#include "Data/RVWeaponDataAsset.h"
#include "Data/RVWeaponStyleDataAsset.h"

// --- Combo (instance-level override supported) --------------------------------

UAnimMontage* URVWeaponDataAsset::GetAttackMontage() const
{
	if (bOverrideCombo) { return OverrideAttackMontage; }
	return IsValid(WeaponStyle) ? WeaponStyle->AttackMontage : nullptr;
}

// --- Heavy Attack (charge and release are overridden independently) -----------

UAnimMontage* URVWeaponDataAsset::GetHeavyChargeMontage() const
{
	if (bOverrideHeavyChargeMontage) { return OverrideHeavyChargeMontage; }
	return IsValid(WeaponStyle) ? WeaponStyle->HeavyChargeMontage : nullptr;
}

UAnimMontage* URVWeaponDataAsset::GetHeavyAttackMontage() const
{
	if (bOverrideHeavyAttackMontage) { return OverrideHeavyAttackMontage; }
	return IsValid(WeaponStyle) ? WeaponStyle->HeavyAttackMontage : nullptr;
}

// --- Dodge (instance-level override supported) --------------------------------

UAnimMontage* URVWeaponDataAsset::GetDodgeMontage() const
{
	if (bOverrideDodgeMontage) { return OverrideDodgeMontage; }
	return IsValid(WeaponStyle) ? WeaponStyle->DodgeMontage : nullptr;
}

// --- Style-level assets (no instance override — WeaponStyle owns these) ------

UAnimMontage* URVWeaponDataAsset::GetGuardBreakMontage() const
{
	return IsValid(WeaponStyle) ? WeaponStyle->GuardBreakMontage : nullptr;
}

UAnimMontage* URVWeaponDataAsset::GetGuardHitMontage() const
{
	return IsValid(WeaponStyle) ? WeaponStyle->GuardHitMontage : nullptr;
}

UBlendSpace* URVWeaponDataAsset::GetLocomotionBS() const
{
	return IsValid(WeaponStyle) ? WeaponStyle->LocomotionBS : nullptr;
}

UBlendSpace* URVWeaponDataAsset::GetLockOnLocomotionBS() const
{
	return IsValid(WeaponStyle) ? WeaponStyle->LockOnLocomotionBS : nullptr;
}

UBlendSpace* URVWeaponDataAsset::GetGuardLocomotionBS() const
{
	return IsValid(WeaponStyle) ? WeaponStyle->GuardLocomotionBS : nullptr;
}

UBlendSpace* URVWeaponDataAsset::GetGuardLocomotionBS_LockOn() const
{
	return IsValid(WeaponStyle) ? WeaponStyle->GuardLocomotionBS_LockOn : nullptr;
}

// --- Combo metadata ----------------------------------------------------------

int32 URVWeaponDataAsset::GetMaxComboCount() const
{
	if (bOverrideCombo) { return OverrideMaxComboCount; }
	return IsValid(WeaponStyle) ? WeaponStyle->MaxComboCount : 0;
}

const TArray<FName>& URVWeaponDataAsset::GetComboSectionNames() const
{
	static const TArray<FName> EmptyNames;
	if (bOverrideCombo) { return OverrideComboSectionNames; }
	return IsValid(WeaponStyle) ? WeaponStyle->ComboSectionNames : EmptyNames;
}