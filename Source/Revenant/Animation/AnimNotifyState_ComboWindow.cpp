#include "Animation/AnimNotifyState_ComboWindow.h"
#include "Component/RVWeaponAttackComponent.h"

void UAnimNotifyState_ComboWindow::NotifyBegin(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner)) { return; }

	URVWeaponAttackComponent* WeaponAttackComp =
		Owner->FindComponentByClass<URVWeaponAttackComponent>();
	if (!IsValid(WeaponAttackComp)) { return; }

	CachedComboComps.Add(MeshComp, WeaponAttackComp);
	WeaponAttackComp->OpenComboWindow();
}

void UAnimNotifyState_ComboWindow::NotifyEnd(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	URVWeaponAttackComponent* WeaponAttackComp = CachedComboComps.FindRef(MeshComp);
	CachedComboComps.Remove(MeshComp);

	if (!IsValid(WeaponAttackComp)) { return; }
	WeaponAttackComp->CloseComboWindow();
}

FString UAnimNotifyState_ComboWindow::GetNotifyName_Implementation() const
{
	return FString(TEXT("ComboWindow"));
}