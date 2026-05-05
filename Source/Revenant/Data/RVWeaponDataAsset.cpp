#include "Data/RVWeaponDataAsset.h"
#include "Data/RVWeaponStyleDataAsset.h"

UAnimMontage* URVWeaponDataAsset::GetAttackMontage() const
{
	if (bOverrideCombo) { return OverrideAttackMontage; }
	return IsValid(WeaponStyle) ? WeaponStyle->AttackMontage : nullptr;
}

UAnimMontage* URVWeaponDataAsset::GetDodgeMontage() const
{
	if (bOverrideDodgeMontage) { return OverrideDodgeMontage; }
	return IsValid(WeaponStyle) ? WeaponStyle->DodgeMontage : nullptr;
}

UAnimMontage* URVWeaponDataAsset::GetGuardBreakMontage() const
{
	if (bOverrideGuardBreakMontage) { return OverrideGuardBreakMontage; }
	return IsValid(WeaponStyle) ? WeaponStyle->GuardBreakMontage : nullptr;
}

UAnimMontage* URVWeaponDataAsset::GetGuardHitMontage() const
{
	if (bOverrideGuardHitMontage) { return OverrideGuardHitMontage; }
	return IsValid(WeaponStyle) ? WeaponStyle->GuardHitMontage : nullptr;
}

UBlendSpace* URVWeaponDataAsset::GetLocomotionBS() const
{
	if (bOverrideLocomotionBS) { return OverrideLocomotionBS; }
	return IsValid(WeaponStyle) ? WeaponStyle->LocomotionBS : nullptr;
}

UBlendSpace* URVWeaponDataAsset::GetGuardLocomotionBS() const
{
	if (bOverrideGuardLocomotionBS) { return OverrideGuardLocomotionBS; }
	return IsValid(WeaponStyle) ? WeaponStyle->GuardLocomotionBS : nullptr;
}

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
