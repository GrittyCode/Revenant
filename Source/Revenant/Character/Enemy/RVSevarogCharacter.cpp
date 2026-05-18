#include "Character/Enemy/RVSevarogCharacter.h"
#include "Animation/AnimInstance.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Data/RVSevarogDataAsset.h"

void ARVSevarogCharacter::BeginPlay()
{
	Super::BeginPlay();

	ensureMsgf(IsValid(SevarogData),
	           TEXT("[%s] SevarogData not assigned"), *GetNameSafe(this));
}

//--- Soul Siphon -------------------------------------------------------------

void ARVSevarogCharacter::ExecuteSoulSiphon()
{
	if (IsGroggy()) { return; }
	if (IsAttacking()) { return; }
	if (!IsValid(SevarogData)) { return; }
	if (!IsValid(SevarogData->SoulSiphonMontage)) { return; }

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInst)) { return; }

	CombatStateComponent->AddState(ERVCombatState::Attacking);
	AnimInst->Montage_Play(SevarogData->SoulSiphonMontage);

	// Heal only if the montage completes without interruption.
	// bInterrupted == true means the player attacked during the channel — no reward.
	FOnMontageBlendingOutStarted BlendOutDelegate;
	BlendOutDelegate.BindLambda([this](UAnimMontage* /*Montage*/, bool bInterrupted)
	{
		CombatStateComponent->RemoveState(ERVCombatState::Attacking);

		const bool bHealed = !bInterrupted;
		if (bHealed)
		{
			AttributeComponent->ApplyHealing(SevarogData->SoulSiphonHealAmount);
		}
		OnSoulSiphonCompleted.Broadcast(bHealed);
	});
	AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, SevarogData->SoulSiphonMontage);
}

//--- Subjugation -------------------------------------------------------------

void ARVSevarogCharacter::ExecuteSubjugation()
{
	if (IsGroggy()) { return; }
	if (IsAttacking()) { return; }
	if (!IsValid(SevarogData)) { return; }
	if (!IsValid(SevarogData->SubjugationMontage)) { return; }

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInst)) { return; }

	CombatStateComponent->AddState(ERVCombatState::Attacking);
	AnimInst->Montage_Play(SevarogData->SubjugationMontage);

	FOnMontageBlendingOutStarted BlendOutDelegate;
	BlendOutDelegate.BindLambda([this](UAnimMontage* /*Montage*/, bool /*bInterrupted*/)
	{
		CombatStateComponent->RemoveState(ERVCombatState::Attacking);
	});
	AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, SevarogData->SubjugationMontage);
}

void ARVSevarogCharacter::SpawnGroundField()
{
	// SpawnActor<ARVGroundField> at boss forward + offset using SevarogData->GroundField* params.
}