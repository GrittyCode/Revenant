#include "Data/RVCombatAnimDataAsset.h"
#include "Animation/AnimMontage.h"

UAnimMontage* URVCombatAnimDataAsset::GetComboMontage(int32 InIndex) const
{
	if (!ComboMontages.IsValidIndex(InIndex)) { return nullptr; }
	return ComboMontages[InIndex].Get();
}

int32 URVCombatAnimDataAsset::FindComboMontageIndex(const UAnimMontage* InMontage) const
{
	for (int32 i = 0; i < ComboMontages.Num(); ++i)
	{
		if (ComboMontages[i].Get() == InMontage) { return i; }
	}
	return INDEX_NONE;
}