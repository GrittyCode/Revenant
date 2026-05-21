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

	// NativeInitializeAnimation fires after BeginPlay, so SevarogData is
	// guaranteed valid by the ensureMsgf in ARVSevarogCharacter::BeginPlay.
	const URVSevarogDataAsset* Data = OwnerSevarog->GetSevarogData();
	if (!IsValid(Data)) { return; }

	// Single 1D BS — direct field access, no LocomotionAnimData wrapper.
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

	// Speed drives the 1D LocomotionBS.
	// CharacterMovement.MaxWalkSpeed switches between normal and rush values —
	// the BS samples the correct pose automatically without a separate bIsRushing flag.
	Speed = OwnerSevarog->GetCharacterMovement()->Velocity.Size();

	bIsInAir         = OwnerSevarog->GetCharacterMovement()->IsFalling();
	bIsAttacking     = CombatStateComponent->HasState(ERVCombatState::Attacking);
	bIsInHitReaction = CombatStateComponent->IsInState(ERVCombatState::HitReaction);
	bIsKnockedDown   = CombatStateComponent->IsInState(ERVCombatState::Knockdown);
	bIsGroggy        = OwnerSevarog->IsGroggy();

	StaggerDirection = HitReactionComponent->GetStaggerDirection();
}