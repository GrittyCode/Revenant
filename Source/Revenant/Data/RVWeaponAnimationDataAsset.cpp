// Source/Revenant/Data/RVWeaponAnimationDataAsset.cpp
#include "Data/RVWeaponAnimationDataAsset.h"

UAnimMontage* URVWeaponAnimationDataAsset::GetComboMontage(int32 InIndex) const
{
	if (!ComboMontages.IsValidIndex(InIndex)) { return nullptr; }
	return ComboMontages[InIndex].Get();
}

int32 URVWeaponAnimationDataAsset::FindComboMontageIndex(const UAnimMontage* InMontage) const
{
	return ComboMontages.IndexOfByPredicate(
		[InMontage](const TObjectPtr<UAnimMontage>& Entry) { return Entry.Get() == InMontage; });
}