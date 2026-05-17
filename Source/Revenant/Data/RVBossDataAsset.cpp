#include "Data/RVBossDataAsset.h"
#include "Data/RVEnemyStatRow.h"

const FRVEnemyStatRow* URVBossDataAsset::GetEnemyStatRow() const
{
	if (EnemyStatRowHandle.IsNull()) { return nullptr; }
	return EnemyStatRowHandle.GetRow<FRVEnemyStatRow>(TEXT("URVBossDataAsset::GetEnemyStatRow"));
}