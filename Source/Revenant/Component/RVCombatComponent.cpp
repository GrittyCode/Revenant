#include "Component/RVCombatComponent.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Component/RVComboComponent.h"
#include "Data/RVWeaponDataAsset.h"
#include "Interface/RVDamageable.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"

URVCombatComponent::URVCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URVCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	OwnerCharacter     = Cast<ACharacter>(Owner);
	AttributeComponent = Owner->FindComponentByClass<URVAttributeComponent>();
	EquipmentComponent = Owner->FindComponentByClass<URVEquipmentComponent>();
	ComboComponent     = Owner->FindComponentByClass<URVComboComponent>();
	if (IsValid(OwnerCharacter))
	{
		MovementComponent = OwnerCharacter->GetCharacterMovement();
	}

	ensureMsgf(IsValid(OwnerCharacter),     TEXT("[%s] Owner is not ACharacter — CombatComponent requires ARVCharacterBase"),       *GetNameSafe(Owner));
	ensureMsgf(IsValid(AttributeComponent), TEXT("[%s] URVAttributeComponent missing — CombatComponent requires ARVCharacterBase"), *GetNameSafe(Owner));
	ensureMsgf(IsValid(EquipmentComponent), TEXT("[%s] URVEquipmentComponent missing — CombatComponent requires ARVCharacterBase"), *GetNameSafe(Owner));
	ensureMsgf(IsValid(ComboComponent),     TEXT("[%s] URVComboComponent missing — CombatComponent requires ARVCharacterBase"),     *GetNameSafe(Owner));
	ensureMsgf(IsValid(MovementComponent),  TEXT("[%s] CharacterMovementComponent missing — owner must be ACharacter subclass"),    *GetNameSafe(Owner));

	AttributeComponent->OnGuardBreak.AddDynamic(this, &URVCombatComponent::OnGuardBreakHandler);
	ComboComponent->OnComboStarted.AddUObject(this, &URVCombatComponent::OnComboStartedHandler);
	ComboComponent->OnComboEnded.AddUObject(this, &URVCombatComponent::OnComboEndedHandler);
}

//--- State -------------------------------------------------------------------

void URVCombatComponent::ForceEndAllActions()
{
	if (HasState(ERVCombatState::HeavyCharging | ERVCombatState::HeavyAttacking)) { EndHeavyAttack(); }
	if (HasState(ERVCombatState::Dodging))                                        { EndDodge(); }
	if (HasState(ERVCombatState::Guarding))                                       { EndGuard(); }
	if (bIsSprinting)                                                             { EndSprint(); }
	RemoveState(ERVCombatState::Attacking);
}

void URVCombatComponent::OnComboStartedHandler()
{
	AddState(ERVCombatState::Attacking);
	// Transition directly from guard to attack — EndGuard is intentionally bypassed
	// to keep stamina regen paused (attack keeps it paused anyway).
	if (HasState(ERVCombatState::Guarding)) { RemoveState(ERVCombatState::Guarding); }
	if (bIsSprinting)                       { EndSprint(); }
}

void URVCombatComponent::OnComboEndedHandler()
{
	RemoveState(ERVCombatState::Attacking);
}

bool URVCombatComponent::IsGrounded() const
{
	return IsValid(MovementComponent) && !MovementComponent->IsFalling();
}

bool URVCombatComponent::CheckAvailableState(ERVCombatState InCoexistableStates) const
{
	const ERVCombatState BlockingStates =
		ERVCombatState::Attacking      |
		ERVCombatState::HeavyCharging  |
		ERVCombatState::HeavyAttacking |
		ERVCombatState::Dodging        |
		ERVCombatState::Guarding       |
		ERVCombatState::GuardBroken;

	const ERVCombatState Relevant = (CurrentStates & BlockingStates) & ~InCoexistableStates;
	return Relevant == ERVCombatState::None;
}

//--- Attack Trace ------------------------------------------------------------

void URVCombatComponent::OpenHitWindow()
{
	HitActors.Empty();
}

void URVCombatComponent::CloseHitWindow()
{
	HitActors.Empty();
}

float URVCombatComponent::ResolveDamage(const URVWeaponDataAsset* InWeaponData) const
{
	if (HasState(ERVCombatState::HeavyAttacking))
	{
		// AutoRelease (max charge) deals maximum damage; manual release uses base heavy damage.
		return (ActiveTier == ERVHeavyAttackTier::AutoRelease)
			? InWeaponData->HeavyAttackDamage_Max
			: InWeaponData->HeavyAttackDamage;
	}
	return InWeaponData->AttackDamage;
}

void URVCombatComponent::PerformAttackTrace()
{
	URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
	if (!IsValid(WeaponData)) { return; }

	USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
	const FVector Root = Mesh->GetSocketLocation(FName("WeaponRoot"));
	const FVector Tip  = Mesh->GetSocketLocation(FName("WeaponTip"));

	const FVector Center     = (Root + Tip) * 0.5f;
	const float   HalfHeight = FVector::Dist(Root, Tip) * 0.5f;
	const FQuat   Rotation   = FRotationMatrix::MakeFromZ(Tip - Root).ToQuat();

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	TArray<FOverlapResult> Overlaps;
	// TODO: replace ECC_Pawn with project-specific channel once RVCollision.h is defined
	GetWorld()->OverlapMultiByChannel(
		Overlaps, Center, Rotation, ECC_Pawn,
		FCollisionShape::MakeCapsule(WeaponData->AttackRadius, HalfHeight),
		Params
	);

#if !UE_BUILD_SHIPPING
	DrawDebugCapsule(GetWorld(), Center, HalfHeight, WeaponData->AttackRadius,
	                 Rotation, FColor::Red, false, 1.f);
#endif

	const float Damage = ResolveDamage(WeaponData);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!IsValid(HitActor)) { continue; }

		TWeakObjectPtr<AActor> WeakHitActor(HitActor);
		if (HitActors.Contains(WeakHitActor)) { continue; }
		HitActors.Add(WeakHitActor);

		if (IRVDamageable* Target = Cast<IRVDamageable>(HitActor))
		{
			Target->ApplyDamage(Damage, OwnerCharacter);
		}
	}
}

//--- Combo -------------------------------------------------------------------

void URVCombatComponent::TryStartCombo()
{
	if (!ComboComponent->IsComboActive())
	{
		if (!IsGrounded()) { return; }
		if (!CheckAvailableState(ERVCombatState::Attacking | ERVCombatState::Guarding)) { return; }
	}

	ComboComponent->HandleComboInput();
}

//--- Heavy Attack ------------------------------------------------------------

void URVCombatComponent::StartHeavyAttack()
{
	if (!CheckAvailableState()) { return; }
	if (!IsGrounded()) { return; }

	const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
	if (!IsValid(WeaponData) || !IsValid(WeaponData->GetHeavyChargeMontage())) { return; }

	// Consume stamina at charge start — cancellation mid-charge is an intended penalty
	if (!AttributeComponent->ConsumeStamina(WeaponData->HeavyAttackStaminaCost)) { return; }

	if (bIsSprinting) { EndSprint(); }

	AddState(ERVCombatState::HeavyCharging);
	bCanHeavyRelease = false;
	bPendingRelease  = false;
	bIsAutoRelease   = false;
	AttributeComponent->PauseStaminaRegen();

	UAnimMontage* ChargeMontage = WeaponData->GetHeavyChargeMontage();
	
	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInstance)) { return; }

	FOnMontageBlendingOutStarted ChargeBlendOutDelegate;
	ChargeBlendOutDelegate.BindUObject(this, &URVCombatComponent::OnChargeMontageBlendingOut);

	AnimInstance->Montage_Play(ChargeMontage);
	AnimInstance->Montage_SetBlendingOutDelegate(ChargeBlendOutDelegate, ChargeMontage);

	GetWorld()->GetTimerManager().SetTimer(
		ChargeAutoReleaseHandle,
		this,
		&URVCombatComponent::OnChargeAutoRelease,
		MaxChargeTime,
		false
	);
}

void URVCombatComponent::ReleaseHeavyAttack()
{
	if (!HasState(ERVCombatState::HeavyCharging)) { return; }

	if (!bCanHeavyRelease)
	{
		// Loop section not yet reached — buffer the release request
		bPendingRelease = true;
		return;
	}

	ExecuteHeavyAttack();
}

void URVCombatComponent::SetHeavyAttackReady(bool bReady)
{
	bCanHeavyRelease = bReady;

	// Flush buffered release — player pressed the button during Begin section
	if (bReady && bPendingRelease)
	{
		bPendingRelease = false;
		ExecuteHeavyAttack();
	}
}

void URVCombatComponent::ExecuteHeavyAttack()
{
	GetWorld()->GetTimerManager().ClearTimer(ChargeAutoReleaseHandle);

	const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
	if (!IsValid(WeaponData))   { EndHeavyAttack(); return; }

	

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInstance)) { EndHeavyAttack(); return; }

	UAnimMontage* ReleaseMontage = WeaponData->GetHeavyAttackMontage();
	if (!IsValid(ReleaseMontage)) { EndHeavyAttack(); return; }

	// Tier is determined by whether the timer fired (auto) or the player released manually.
	ActiveTier     = bIsAutoRelease ? ERVHeavyAttackTier::AutoRelease : ERVHeavyAttackTier::Manual;
	bIsAutoRelease = false;

	// Transition state before stopping charge montage.
	// OnChargeMontageBlendingOut checks HeavyCharging to detect external interruptions —
	// removing it here tells that handler this stop is intentional.
	RemoveState(ERVCombatState::HeavyCharging);
	AddState(ERVCombatState::HeavyAttacking);

	FOnMontageBlendingOutStarted ReleaseBlendOutDelegate;
	ReleaseBlendOutDelegate.BindUObject(this, &URVCombatComponent::OnReleaseMontageBlendingOut);

	AnimInstance->Montage_Play(ReleaseMontage);
	AnimInstance->Montage_SetBlendingOutDelegate(ReleaseBlendOutDelegate, ReleaseMontage);
}

void URVCombatComponent::OnChargeAutoRelease()
{
	// Max charge time reached — flag as auto-release before delegating.
	// ExecuteHeavyAttack reads this flag to assign AutoRelease tier (max damage).
	UE_LOG(LogTemp, Log, TEXT("[%s] URVCombatComponent: OnChargeAutoRelease fired"), *GetNameSafe(GetOwner()));
	bIsAutoRelease = true;
	ReleaseHeavyAttack();
}

void URVCombatComponent::EndHeavyAttack()
{
	if (!HasState(ERVCombatState::HeavyCharging | ERVCombatState::HeavyAttacking)) { return; }

	// Stop whichever montage is currently active so visual and logical state stay in sync.

	if (IsValid(OwnerCharacter))
	{
		UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
		if (IsValid(AnimInstance))
		{
			const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
			if (IsValid(WeaponData))
			{
				UAnimMontage* MontageToStop = HasState(ERVCombatState::HeavyCharging)
					? WeaponData->GetHeavyChargeMontage()
					: WeaponData->GetHeavyAttackMontage();
				AnimInstance->Montage_Stop(0.1f, MontageToStop);
			}
		}
	}

	RemoveState(ERVCombatState::HeavyCharging | ERVCombatState::HeavyAttacking);
	bCanHeavyRelease = false;
	bPendingRelease  = false;
	bIsAutoRelease   = false;
	GetWorld()->GetTimerManager().ClearTimer(ChargeAutoReleaseHandle);
	AttributeComponent->ResumeStaminaRegen();
}

void URVCombatComponent::OnChargeMontageBlendingOut(UAnimMontage* /*InMontage*/, bool /*bInterrupted*/)
{
	// HeavyCharging still set = external interruption before release — clean up.
	// HeavyCharging already removed = ExecuteHeavyAttack transitioned us — ignore.
	if (HasState(ERVCombatState::HeavyCharging))
	{
		EndHeavyAttack();
	}
}

void URVCombatComponent::OnReleaseMontageBlendingOut(UAnimMontage* /*InMontage*/, bool /*bInterrupted*/)
{
	EndHeavyAttack();
}

//--- Dodge -------------------------------------------------------------------

void URVCombatComponent::StartDodge(const FVector& InDodgeDirection)
{
	if (!CheckAvailableState(ERVCombatState::Guarding)) { return; }
	if (!IsGrounded()) { return; }

	const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
	if (!IsValid(WeaponData) || !IsValid(WeaponData->GetDodgeMontage())) { return; }

	if (!AttributeComponent->ConsumeStamina(WeaponData->DodgeStaminaCost)) { return; }

	if (bIsSprinting)                       { EndSprint(); }
	if (HasState(ERVCombatState::Guarding)) { EndGuard(); }

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInstance)) { return; }

	OwnerCharacter->SetActorRotation(InDodgeDirection.ToOrientationRotator());
	MovementComponent->bOrientRotationToMovement = false;

	AddState(ERVCombatState::Dodging);
	AttributeComponent->PauseStaminaRegen();

	UAnimMontage* DodgeMontage = WeaponData->GetDodgeMontage();

	FOnMontageBlendingOutStarted BlendOutDelegate;
	BlendOutDelegate.BindUObject(this, &URVCombatComponent::OnDodgeMontageBlendingOut);

	AnimInstance->Montage_Play(DodgeMontage);
	AnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, DodgeMontage);
}

void URVCombatComponent::SetDodgeIFrame(bool bActivate)
{
	if (!HasState(ERVCombatState::Dodging) && bActivate) { return; }
	bIsInvincible = bActivate;
}

void URVCombatComponent::EndDodge()
{
	if (!HasState(ERVCombatState::Dodging)) { return; }

	RemoveState(ERVCombatState::Dodging);
	bIsInvincible = false;
	MovementComponent->bOrientRotationToMovement = true;
	AttributeComponent->ResumeStaminaRegen();
}

void URVCombatComponent::OnDodgeMontageBlendingOut(UAnimMontage* /*InMontage*/, bool /*bInterrupted*/)
{
	EndDodge();
}

//--- Guard -------------------------------------------------------------------

void URVCombatComponent::StartGuard()
{
	if (!CheckAvailableState()) { return; }
	if (!IsGrounded()) { return; }
	if (bIsSprinting) { EndSprint(); }

	AddState(ERVCombatState::Guarding);
	AttributeComponent->PauseStaminaRegen();
}

void URVCombatComponent::EndGuard()
{
	if (!HasState(ERVCombatState::Guarding)) { return; }

	RemoveState(ERVCombatState::Guarding);
	AttributeComponent->ResumeStaminaRegen();
}

void URVCombatComponent::HandleGuardHit(float InDamageAmount)
{
	const bool bGuardHeld = AttributeComponent->ApplyStaminaDamage(InDamageAmount);
	if (!bGuardHeld) { return; }

	const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
	if (!IsValid(WeaponData) || !IsValid(WeaponData->GetGuardHitMontage())) { return; }

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInstance)) { return; }

	AnimInstance->Montage_Play(WeaponData->GetGuardHitMontage());
}

//--- Guard Break -------------------------------------------------------------

void URVCombatComponent::OnGuardBreakHandler()
{
	RemoveState(ERVCombatState::Guarding);
	AddState(ERVCombatState::GuardBroken);
	AttributeComponent->PauseStaminaRegen();

	URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
	if (!IsValid(WeaponData) || !IsValid(WeaponData->GetGuardBreakMontage())) { return; }

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInstance)) { return; }

	AnimInstance->Montage_Play(WeaponData->GetGuardBreakMontage());

	GetWorld()->GetTimerManager().SetTimer(
		GuardBreakRecoveryHandle,
		this,
		&URVCombatComponent::OnGuardBreakRecoveryComplete,
		GuardBreakRecoveryTime,
		false
	);
}

void URVCombatComponent::OnGuardBreakRecoveryComplete()
{
	RemoveState(ERVCombatState::GuardBroken);
	AttributeComponent->ResumeStaminaRegen();
}

//--- Sprint ------------------------------------------------------------------

void URVCombatComponent::StartSprint()
{
	if (bIsSprinting) { return; }
	if (!CheckAvailableState()) { return; }
	if (!IsGrounded()) { return; }
	if (AttributeComponent->GetCurrentStamina() <= 0.f) { return; }

	OriginalWalkSpeed = MovementComponent->MaxWalkSpeed;
	MovementComponent->MaxWalkSpeed = SprintSpeed;
	bIsSprinting = true;
}

void URVCombatComponent::EndSprint()
{
	if (!bIsSprinting) { return; }

	bIsSprinting = false;
	MovementComponent->MaxWalkSpeed = OriginalWalkSpeed;
}