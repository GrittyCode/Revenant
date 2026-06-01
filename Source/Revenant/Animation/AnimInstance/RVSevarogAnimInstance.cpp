#include "Animation/AnimInstance/RVSevarogAnimInstance.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "Data/Asset/RVSevarogDataAsset.h"
#include "Data/Asset/RVHitReactionAnimDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"

void URVSevarogAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerSevarog = Cast<ARVSevarogCharacter>(GetOwningActor());
	ensureMsgf(IsValid(OwnerSevarog),
		TEXT("[URVSevarogAnimInstance] Owner is not ARVSevarogCharacter — check ABP assignment"));
	if (!IsValid(OwnerSevarog)) { return; }

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

	const UWorld* W = GetWorld();
	if (!W || !W->IsGameWorld()) { return; }

	Speed = OwnerSevarog->GetCharacterMovement()->Velocity.Size2D();

	bIsAttacking     = OwnerSevarog->IsAttacking();
	bIsInHitReaction = OwnerSevarog->IsInHitReaction();
	bIsKnockedDown   = OwnerSevarog->IsKnockedDown();
	bIsGroggy        = OwnerSevarog->IsGroggy();

	StaggerDirection = OwnerSevarog->GetStaggerDirectionForAnim();
}