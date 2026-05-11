#include "Data/RVMontageStatData.h"
#include "Data/RVAttackActionMultiplierRow.h"

const FRVAttackActionMultiplierRow* URVMontageStatData::GetStatRow() const
{
	return AttackStatRowHandle.GetRow<FRVAttackActionMultiplierRow>(TEXT("URVMontageStatData::GetStatRow"));
}