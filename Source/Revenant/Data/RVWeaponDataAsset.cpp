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

int32 URVWeaponDataAsset::FindComboMontageIndex(const UAnimMontage* InMontage) const
{
    return IsValid(AnimationDataAsset) ? AnimationDataAsset->FindComboMontageIndex(InMontage) : INDEX_NONE;
}

UAnimMontage* URVWeaponDataAsset::GetHeavyChargeMontage() const
{
    if (bOverrideHeavyChargeMontage) { return OverrideHeavyChargeMontage; }
    return IsValid(AnimationDataAsset) ? AnimationDataAsset->HeavyChargeMontage.Get() : nullptr;
}

UAnimMontage* URVWeaponDataAsset::GetHeavyAttackMontage(bool bIsMax) const
{
    if (bOverrideHeavyAttackMontage)
    {
        return bIsMax ? OverrideMaxHeavyAttackMontage.Get() : OverrideHeavyAttackMontage.Get();
    }
    if (!IsValid(AnimationDataAsset)) { return nullptr; }
    return bIsMax ? AnimationDataAsset->MaxHeavyAttackMontage.Get()
                  : AnimationDataAsset->HeavyAttackMontage.Get();
}

UAnimMontage* URVWeaponDataAsset::GetDodgeMontage() const
{
    if (bOverrideDodgeMontage) { return OverrideDodgeMontage; }
    return IsValid(AnimationDataAsset) ? AnimationDataAsset->DodgeMontage.Get() : nullptr;
}

UAnimMontage* URVWeaponDataAsset::GetGuardBreakMontage() const
{
    return IsValid(AnimationDataAsset) ? AnimationDataAsset->GuardBreakMontage.Get() : nullptr;
}

UAnimMontage* URVWeaponDataAsset::GetGuardHitMontage() const
{
    return IsValid(AnimationDataAsset) ? AnimationDataAsset->GuardHitMontage.Get() : nullptr;
}

UAnimMontage* URVWeaponDataAsset::GetGroggyMontage() const
{
    return IsValid(AnimationDataAsset) ? AnimationDataAsset->GroggyMontage.Get() : nullptr;
}

UAnimMontage* URVWeaponDataAsset::GetKnockdownMontage() const
{
    return IsValid(AnimationDataAsset) ? AnimationDataAsset->KnockdownMontage.Get() : nullptr;
}

UAnimMontage* URVWeaponDataAsset::GetGetUpMontage() const
{
    return IsValid(AnimationDataAsset) ? AnimationDataAsset->GetUpMontage.Get() : nullptr;
}

UBlendSpace* URVWeaponDataAsset::GetStaggerBlendSpace() const
{
    return IsValid(AnimationDataAsset) ? AnimationDataAsset->StaggerBlendSpace.Get() : nullptr;
}

UBlendSpace* URVWeaponDataAsset::GetLocomotionBS() const
{
    return IsValid(AnimationDataAsset) ? AnimationDataAsset->LocomotionBS.Get() : nullptr;
}

UBlendSpace* URVWeaponDataAsset::GetLockOnLocomotionBS() const
{
    return IsValid(AnimationDataAsset) ? AnimationDataAsset->LockOnLocomotionBS.Get() : nullptr;
}

UBlendSpace* URVWeaponDataAsset::GetGuardLocomotionBS() const
{
    return IsValid(AnimationDataAsset) ? AnimationDataAsset->GuardLocomotionBS.Get() : nullptr;
}

UBlendSpace* URVWeaponDataAsset::GetGuardLocomotionBS_LockOn() const
{
    return IsValid(AnimationDataAsset) ? AnimationDataAsset->GuardLocomotionBS_LockOn.Get() : nullptr;
}