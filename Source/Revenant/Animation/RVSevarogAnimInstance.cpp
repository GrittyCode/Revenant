#include "Animation/RVSevarogAnimInstance.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVHitReactionComponent.h"
#include "Data/RVSevarogDataAsset.h"
#include "Data/RVHitReactionAnimDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"

void URVSevarogAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerSevarog = Cast<ARVSevarogCharacter>(GetOwningActor());
	if (!IsValid(OwnerSevarog)) { return; }

	CombatStateComponent = OwnerSevarog->FindComponentByClass<URVCombatStateComponent>();
	HitReactionComponent = OwnerSevarog->FindComponentByClass<URVHitReactionComponent>();

	ensureMsgf(IsValid(CombatStateComponent),
		TEXT("[%s] CombatStateComponent missing"), *GetNameSafe(OwnerSevarog));
	ensureMsgf(IsValid(HitReactionComponent),
		TEXT("[%s] HitReactionComponent missing"), *GetNameSafe(OwnerSevarog));

	const URVSevarogDataAsset* Data = OwnerSevarog->GetSevarogData();
	if (!IsValid(Data)) { return; }

	CachedLocomotionBS = Data->LocomotionBS;

	if (IsValid(Data->HitReactionAnimData))
	{
		CachedStaggerBlendSpace = Data->HitReactionAnimData->StaggerBlendSpace;
	}
}

void URVSevarogAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!IsValid(OwnerSevarog)) { return; }
	if (!IsValid(CombatStateComponent) || !IsValid(HitReactionComponent)) { return; }

	Speed = OwnerSevarog->GetCharacterMovement()->Velocity.Size();

	bIsAttacking     = CombatStateComponent->HasState(ERVCombatState::Attacking);
	bIsInHitReaction = CombatStateComponent->HasState(ERVCombatState::HitReaction);
	bIsKnockedDown   = CombatStateComponent->HasState(ERVCombatState::Knockdown);
	bIsGroggy        = OwnerSevarog->IsGroggy();

	StaggerDirection = HitReactionComponent->GetStaggerDirection();
}