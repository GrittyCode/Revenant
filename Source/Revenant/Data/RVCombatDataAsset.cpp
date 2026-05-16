#include "Data/RVCombatDataAsset.h"
#include "Data/RVWeaponStatRow.h"

const FRVWeaponStatRow* URVCombatDataAsset::GetWeaponStatRow() const
{
	if (WeaponStatRowHandle.IsNull()) { return nullptr; }
	return WeaponStatRowHandle.GetRow<FRVWeaponStatRow>(TEXT("URVCombatDataAsset"));
}