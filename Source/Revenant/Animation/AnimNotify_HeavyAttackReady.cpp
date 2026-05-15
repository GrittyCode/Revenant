#include "Animation/AnimNotify_HeavyAttackReady.h"
#include "Component/RVHeavyAttackComponent.h"

void UAnimNotify_HeavyAttackReady::Notify(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner)) { return; }

	URVHeavyAttackComponent* HeavyAttackComp =
		Owner->FindComponentByClass<URVHeavyAttackComponent>();
	if (!IsValid(HeavyAttackComp)) { return; }

	HeavyAttackComp->SetHeavyAttackReady(true);
}

FString UAnimNotify_HeavyAttackReady::GetNotifyName_Implementation() const
{
	return FString(TEXT("HeavyAttackReady"));
}