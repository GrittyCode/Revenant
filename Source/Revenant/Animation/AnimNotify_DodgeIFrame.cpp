// Source/Revenant/Animation/AnimNotify_DodgeIFrame.cpp
#include "Animation/AnimNotify_DodgeIFrame.h"
#include "Component/RVCombatComponent.h"

void UAnimNotify_DodgeIFrame::Notify(USkeletalMeshComponent*         MeshComp,
									 UAnimSequenceBase*               Animation,
									 const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	// MeshComp and its owner are guaranteed valid at this call site — no IsValid check needed
	URVCombatComponent* CombatComp =
		MeshComp->GetOwner()->FindComponentByClass<URVCombatComponent>();

	if (!IsValid(CombatComp)) { return; }

	CombatComp->SetDodgeIFrame(bActivate);
}