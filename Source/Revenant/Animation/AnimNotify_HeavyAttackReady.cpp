#include "Animation/AnimNotify_HeavyAttackReady.h"
#include "Component/RVWeaponAttackComponent.h"

void UAnimNotify_HeavyAttackReady::Notify(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner)) { return; }

	URVWeaponAttackComponent* WeaponAttackComp =
		Owner->FindComponentByClass<URVWeaponAttackComponent>();
	if (!IsValid(WeaponAttackComp)) { return; }

	WeaponAttackComp->SetHeavyAttackReady(true);
}

FString UAnimNotify_HeavyAttackReady::GetNotifyName_Implementation() const
{
	return FString(TEXT("HeavyAttackReady"));
}