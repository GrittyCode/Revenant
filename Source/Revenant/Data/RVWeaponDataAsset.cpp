// Source/Revenant/Data/RVWeaponDataAsset.cpp
#include "Data/RVWeaponDataAsset.h"
#include "Data/RVWeaponAnimationDataAsset.h"
#include "Data/RVWeaponStatRow.h"

const FRVWeaponStatRow* URVWeaponDataAsset::GetWeaponStatRow() const
{
    return WeaponStatRowHandle.GetRow<FRVWeaponStatRow>(TEXT("URVWeaponDataAsset::GetWeaponStatRow"));
}

UAnimMontage* URVWeaponDataAsset::GetComboMontage(int32 InIndex) const
{
    return IsValid(AnimationDataAsset) ? AnimationDataAsset->GetComboMontage(InIndex) : nullptr;
}

int32 URVWeaponDataAsset::GetMaxComboCount() const
{
    return IsValid(AnimationDataAsset) ? AnimationDataAsset->GetMaxComboCount() : 0;
}

UAnimMontage* URVWeaponDataAsset::GetHeavyChargeMontage() const
{
    if (bOverrideHeavyChargeMontage) { return OverrideHeavyChargeMontage; }
    return IsValid(AnimationDataAsset) ? AnimationDataAsset->HeavyChargeMontage : nullptr;
}

UAnimMontage* URVWeaponDataAsset::GetHeavyAttackMontage(bool bIsMax) const
{
    if (bOverrideHeavyAttackMontage)
    {
        return bIsMax
            ? OverrideMaxHeavyAttackMontage.Get()
            : OverrideHeavyAttackMontage.Get();
    }
    if (!IsValid(AnimationDataAsset)) { return nullptr; }
    return bIsMax
        ? AnimationDataAsset->HeavyAttackMontage.Get()
        : AnimationDataAsset->MaxHeavyAttackMontage.Get();
}

UAnimMontage* URVWeaponDataAsset::GetDodgeMontage() const
{
    if (bOverrideDodgeMontage) { return OverrideDodgeMontage; }
    return IsValid(AnimationDataAsset) ? AnimationDataAsset->DodgeMontage : nullptr;
}

UAnimMontage* URVWeaponDataAsset::GetGuardBreakMontage() const
{
    return IsValid(AnimationDataAsset) ? AnimationDataAsset->GuardBreakMontage : nullptr;
}

UAnimMontage* URVWeaponDataAsset::GetGuardHitMontage() const
{
    return IsValid(AnimationDataAsset) ? AnimationDataAsset->GuardHitMontage : nullptr;
}

UBlendSpace* URVWeaponDataAsset::GetLocomotionBS() const
{
    return IsValid(AnimationDataAsset) ? AnimationDataAsset->LocomotionBS : nullptr;
}

UBlendSpace* URVWeaponDataAsset::GetLockOnLocomotionBS() const
{
    return IsValid(AnimationDataAsset) ? AnimationDataAsset->LockOnLocomotionBS : nullptr;
}

UBlendSpace* URVWeaponDataAsset::GetGuardLocomotionBS() const
{
    return IsValid(AnimationDataAsset) ? AnimationDataAsset->GuardLocomotionBS : nullptr;
}

UBlendSpace* URVWeaponDataAsset::GetGuardLocomotionBS_LockOn() const
{
    return IsValid(AnimationDataAsset) ? AnimationDataAsset->GuardLocomotionBS_LockOn : nullptr;
}