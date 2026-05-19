#include "Data/RVWeaponDataAsset.h"
#include "Data/RVPlayerCombatAnimDataAsset.h"
#include "Data/RVWeaponStatRow.h"

const FRVWeaponStatRow* URVWeaponDataAsset::GetWeaponStatRow() const
{
	if (WeaponStatRowHandle.IsNull()) { return nullptr; }
	return WeaponStatRowHandle.GetRow<FRVWeaponStatRow>(TEXT("URVWeaponDataAsset::GetWeaponStatRow"));
}

UAnimMontage* URVWeaponDataAsset::GetHeavyChargeMontage() const
{
	if (bOverrideHeavyChargeMontage && IsValid(OverrideHeavyChargeMontage))
	{
		return OverrideHeavyChargeMontage;
	}
	return CombatAnimData->HeavyChargeMontage;
}

UAnimMontage* URVWeaponDataAsset::GetHeavyAttackMontage(bool bIsMax) const
{
	if (bOverrideHeavyAttackMontage)
	{
		return bIsMax ? OverrideMaxHeavyAttackMontage : OverrideHeavyAttackMontage;
	}
	return bIsMax ? CombatAnimData->MaxHeavyAttackMontage
	              : CombatAnimData->HeavyAttackMontage;
}

UAnimMontage* URVWeaponDataAsset::GetDodgeMontage() const
{
	if (bOverrideDodgeMontage && IsValid(OverrideDodgeMontage)) { return OverrideDodgeMontage; }
	return CombatAnimData->DodgeMontage;
}

UAnimMontage* URVWeaponDataAsset::GetLockOnDodgeMontage(ERVDodgeDirection InDirection) const
{
	if (!IsValid(CombatAnimData)) { return GetDodgeMontage(); }

	const TObjectPtr<UAnimMontage>* Found = CombatAnimData->LockOnDodgeMontages.Find(InDirection);
	if (Found && IsValid(*Found))
	{
		return Found->Get();
	}
	return GetDodgeMontage();
}
