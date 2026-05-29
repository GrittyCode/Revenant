#include "Animation/AnimInstance/RVDummyAnimInstance.h"
#include "Character/Base/RVCharacterBase.h"
#include "Component/Combat/RVCombatStateComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

void URVDummyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	CachedOwner = Cast<ARVCharacterBase>(GetOwningActor());
}

void URVDummyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!CachedOwner.IsValid()) { return; }

	const UCharacterMovementComponent* CMC = CachedOwner->GetCharacterMovement();
	Speed = CMC ? CMC->Velocity.Size2D() : 0.f;

	// HitReaction covers stagger. Knockdown montage overrides AnimGraph output
	// so no need to exclude ERVCombatState::Knockdown here.
	bIsStaggering    = CachedOwner->HasCombatState(ERVCombatState::HitReaction);
	StaggerDirection = CachedOwner->GetStaggerDirection();
}