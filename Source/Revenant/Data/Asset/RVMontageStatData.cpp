#include "Data/Asset/RVMontageStatData.h"
#include "Data/Row/RVAttackActionMultiplierRow.h"

const FRVAttackActionMultiplierRow* URVMontageStatData::GetStatRow() const
{
	return AttackStatRowHandle.GetRow<FRVAttackActionMultiplierRow>(TEXT("URVMontageStatData::GetStatRow"));
}