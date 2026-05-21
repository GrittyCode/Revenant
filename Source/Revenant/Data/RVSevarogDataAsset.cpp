#include "Data/RVSevarogDataAsset.h"
#include "Data/RVEnemyStatRow.h"

const FRVEnemyStatRow* URVSevarogDataAsset::GetEnemyStatRow() const
{
	if (EnemyStatRowHandle.IsNull()) { return nullptr; }
	return EnemyStatRowHandle.GetRow<FRVEnemyStatRow>(TEXT("URVSevarogDataAsset::GetEnemyStatRow"));
}
