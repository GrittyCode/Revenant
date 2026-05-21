#include "Data/RVSevarogDataAsset.h"
#include "Data/RVEnemyStatRow.h"
#include "Data/RVHitReactionAnimDataAsset.h"

const FRVEnemyStatRow* URVSevarogDataAsset::GetEnemyStatRow() const
{
	if (EnemyStatRowHandle.IsNull()) { return nullptr; }
	return EnemyStatRowHandle.GetRow<FRVEnemyStatRow>(TEXT("URVSevarogDataAsset::GetEnemyStatRow"));
}

UAnimMontage* URVSevarogDataAsset::GetGroggyStunStartMontage() const
{
	return IsValid(HitReactionAnimData) ? HitReactionAnimData->GroggyStunStartMontage.Get() : nullptr;
}

UAnimMontage* URVSevarogDataAsset::GetGroggyStunLoopMontage() const
{
	return IsValid(HitReactionAnimData) ? HitReactionAnimData->GroggyStunLoopMontage.Get() : nullptr;
}

UAnimMontage* URVSevarogDataAsset::GetGroggyStunEndMontage() const
{
	return IsValid(HitReactionAnimData) ? HitReactionAnimData->GroggyStunEndMontage.Get() : nullptr;
}