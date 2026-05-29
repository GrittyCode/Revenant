#include "Character/Enemy/RVDummyTarget.h"
#include "Component/Combat/RVHitReactionComponent.h"
#include "Component/Attribute/RVVitalComponent.h"
#include "Data/Asset/RVHitReactionAnimDataAsset.h"
#include "Data/Row/RVCharacterStatRow.h"

ARVDummyTarget::ARVDummyTarget()
{
	PrimaryActorTick.bCanEverTick = false;

	GetMesh()->SetRelativeLocationAndRotation(
		FVector(0.f, 0.f, -88.f),
		FRotator(0.f, -90.f, 0.f));
}

//--- ARVCharacterBase overrides ----------------------------------------------

void ARVDummyTarget::InitStats()
{
	const FRVCharacterStatRow* StatRow =
		DummyStatRowHandle.GetRow<FRVCharacterStatRow>(TEXT("ARVDummyTarget::InitStats"));

	if (!ensureMsgf(StatRow,
		TEXT("[%s] InitStats: DummyStatRowHandle not assigned or row missing — assign DT_DummyStats/Dummy_01 in BP_DummyTarget"),
		*GetName())) { return; }

	VitalComponent->InitFromStatRow(*StatRow);

	HitReactionComponent->InitParams(
		GetHitReactionAnimData(),
		StatRow->StaggerDuration,
		StatRow->StaggerThreshold,
		StatRow->KnockdownThreshold);
}

URVHitReactionAnimDataAsset* ARVDummyTarget::GetHitReactionAnimData() const
{
	return HitReactionData;
}