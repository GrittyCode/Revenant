#include "Animation/AnimNotify_ComboChain.h"
#include "Component/RVWeaponAttackComponent.h"

void UAnimNotify_ComboChain::Notify(USkeletalMeshComponent* MeshComp,
									UAnimSequenceBase* Animation,
									const FAnimNotifyEventReference& EventReference)
{
	AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner)) { return; }

	URVWeaponAttackComponent* WeaponAttackComp =
		Owner->FindComponentByClass<URVWeaponAttackComponent>();
	if (!IsValid(WeaponAttackComp)) { return; }

	WeaponAttackComp->TryChainNextCombo();
}