#include "Data/RVWeaponDataAsset.h"
#include "Data/RVCombatAnimDataAsset.h"
#include "Data/RVCombatDataAsset.h"

const FRVWeaponStatRow* URVWeaponDataAsset::GetWeaponStatRow() const
{
    return CombatData->GetWeaponStatRow();
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

UAnimMontage* URVWeaponDataAsset::GetDodgeMontage_LockOn_F() const
{
    return IsValid(CombatAnimData->LockOnDodgeMontage_F)
        ? CombatAnimData->LockOnDodgeMontage_F.Get() : GetDodgeMontage();
}

UAnimMontage* URVWeaponDataAsset::GetDodgeMontage_LockOn_L() const
{
    return IsValid(CombatAnimData->LockOnDodgeMontage_L)
        ? CombatAnimData->LockOnDodgeMontage_L.Get() : GetDodgeMontage();
}

UAnimMontage* URVWeaponDataAsset::GetDodgeMontage_LockOn_R() const
{
    return IsValid(CombatAnimData->LockOnDodgeMontage_R)
        ? CombatAnimData->LockOnDodgeMontage_R.Get() : GetDodgeMontage();
}

UAnimMontage* URVWeaponDataAsset::GetDodgeMontage_LockOn_BL() const
{
    return IsValid(CombatAnimData->LockOnDodgeMontage_BL)
        ? CombatAnimData->LockOnDodgeMontage_BL.Get() : GetDodgeMontage();
}

UAnimMontage* URVWeaponDataAsset::GetDodgeMontage_LockOn_BR() const
{
    return IsValid(CombatAnimData->LockOnDodgeMontage_BR)
        ? CombatAnimData->LockOnDodgeMontage_BR.Get() : GetDodgeMontage();
}