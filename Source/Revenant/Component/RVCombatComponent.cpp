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
	AttributeComponent = Owner->FindComponentByClass<URVAttributeComponent>();
	EquipmentComponent = Owner->FindComponentByClass<URVEquipmentComponent>();
	ComboComponent     = Owner->FindComponentByClass<URVComboComponent>();

	ACharacter* OwnerChar = Cast<ACharacter>(Owner);
	if (IsValid(OwnerChar))
	{
		MovementComponent = OwnerChar->GetCharacterMovement();
	}

	ensureMsgf(IsValid(AttributeComponent), TEXT("[%s] URVAttributeComponent missing — CombatComponent requires ARVCharacterBase"), *GetNameSafe(Owner));
	ensureMsgf(IsValid(EquipmentComponent), TEXT("[%s] URVEquipmentComponent missing — CombatComponent requires ARVCharacterBase"), *GetNameSafe(Owner));
	ensureMsgf(IsValid(ComboComponent),     TEXT("[%s] URVComboComponent missing — CombatComponent requires ARVCharacterBase"),     *GetNameSafe(Owner));
	ensureMsgf(IsValid(MovementComponent),  TEXT("[%s] CharacterMovementComponent missing — owner must be ACharacter subclass"),    *GetNameSafe(Owner));

	AttributeComponent->OnGuardBreak.AddDynamic(this, &URVCombatComponent::OnGuardBreakHandler);
	ComboComponent->OnComboStarted.AddUObject(this, &URVCombatComponent::OnComboStartedHandler);
	ComboComponent->OnComboEnded.AddUObject(this, &URVCombatComponent::OnComboEndedHandler);
}

//--- State -------------------------------------------------------------------

void URVCombatComponent::SetAttacking(bool bInIsAttacking)
{
	bIsAttacking = bInIsAttacking;
}

void URVCombatComponent::ForceEndAllActions()
{
	if (bIsHeavyCharging || bIsHeavyAttacking) { EndHeavyAttack(); }
	if (bIsDodging)                            { EndDodge(); }
	if (bIsGuarding)                           { EndGuard(); }
	if (bIsSprinting)                          { EndSprint(); }
	if (bIsAttacking)                          { SetAttacking(false); }
}

void URVCombatComponent::OnComboStartedHandler()
{
	SetAttacking(true);
	if (bIsGuarding)  { bIsGuarding = false; }
	if (bIsSprinting) { EndSprint(); }
}

void URVCombatComponent::OnComboEndedHandler()
{
	SetAttacking(false);
}

bool URVCombatComponent::IsGrounded() const
{
	return IsValid(MovementComponent) && !MovementComponent->IsFalling();
}

ERVCombatState URVCombatComponent::GetActiveStates() const
{
	ERVCombatState States = ERVCombatState::None;
	if (bIsAttacking)                          { States |= ERVCombatState::Attacking; }
	if (bIsHeavyCharging || bIsHeavyAttacking) { States |= ERVCombatState::HeavyAttacking; }
	if (bIsDodging)                            { States |= ERVCombatState::Dodging; }
	if (bIsGuarding)                           { States |= ERVCombatState::Guarding; }
	if (bIsGuardBroken)                        { States |= ERVCombatState::GuardBroken; }
	return States;
}

bool URVCombatComponent::CanPerformActionWith(ERVCombatState InCoexistableStates) const
{
	const ERVCombatState BlockingStates =
		ERVCombatState::Attacking      |
		ERVCombatState::HeavyAttacking |
		ERVCombatState::Dodging        |
		ERVCombatState::Guarding       |
		ERVCombatState::GuardBroken;

	const ERVCombatState Relevant = (GetActiveStates() & BlockingStates) & ~InCoexistableStates;
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

void URVCombatComponent::PerformAttackTrace()
{
	URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
	if (!IsValid(WeaponData)) { return; }

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!IsValid(OwnerChar)) { return; }

	USkeletalMeshComponent* Mesh = OwnerChar->GetMesh();
	const FVector Root = Mesh->GetSocketLocation(FName("WeaponRoot"));
	const FVector Tip  = Mesh->GetSocketLocation(FName("WeaponTip"));

	const FVector Center     = (Root + Tip) * 0.5f;
	const float   HalfHeight = FVector::Dist(Root, Tip) * 0.5f;
	const FQuat   Rotation   = FRotationMatrix::MakeFromZ(Tip - Root).ToQuat();

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerChar);

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

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!IsValid(HitActor)) { continue; }

		TWeakObjectPtr<AActor> WeakHitActor(HitActor);
		if (HitActors.Contains(WeakHitActor)) { continue; }
		HitActors.Add(WeakHitActor);

		if (IRVDamageable* Target = Cast<IRVDamageable>(HitActor))
		{
			float Damage = WeaponData->AttackDamage;

			if (bIsHeavyAttacking)
			{
				// AutoRelease (max charge) deals maximum damage; manual release uses base heavy damage.
				Damage = (ActiveTier == ERVHeavyAttackTier::AutoRelease)
					? WeaponData->HeavyAttackDamage_Max
					: WeaponData->HeavyAttackDamage;
			}

			Target->ApplyDamage(Damage, OwnerChar);
		}
	}
}

//--- Combo -------------------------------------------------------------------

void URVCombatComponent::TryStartCombo()
{
	if (!ComboComponent->IsComboActive())
	{
		if (!IsGrounded()) { return; }
		if (!CanPerformActionWith(ERVCombatState::Attacking | ERVCombatState::Guarding)) { return; }
	}

	ComboComponent->HandleComboInput();
}

//--- Heavy Attack ------------------------------------------------------------

void URVCombatComponent::StartHeavyAttack()
{
	if (!CanPerformActionWith()) { return; }
	if (!IsGrounded()) { return; }

	const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
	if (!IsValid(WeaponData) || !IsValid(WeaponData->GetHeavyChargeMontage())) { return; }

	// Consume stamina at charge start — cancellation mid-charge is an intended penalty
	if (!AttributeComponent->ConsumeStamina(WeaponData->HeavyAttackStaminaCost)) { return; }

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!IsValid(OwnerChar)) { return; }

	UAnimInstance* AnimInstance = OwnerChar->GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInstance)) { return; }

	if (bIsSprinting) { EndSprint(); }

	bIsHeavyCharging  = true;
	bCanHeavyRelease  = false;  // gated until AnimNotify_HeavyAttackReady fires
	bPendingRelease   = false;
	bIsAutoRelease    = false;
	AttributeComponent->PauseStaminaRegen();

	UAnimMontage* ChargeMontage = WeaponData->GetHeavyChargeMontage();

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
	if (!bIsHeavyCharging) { return; }

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
	// Stamina was already consumed at StartHeavyAttack — no second charge here

	GetWorld()->GetTimerManager().ClearTimer(ChargeAutoReleaseHandle);

	const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
	if (!IsValid(WeaponData))
	{
		EndHeavyAttack();
		return;
	}

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!IsValid(OwnerChar))
	{
		EndHeavyAttack();
		return;
	}

	UAnimInstance* AnimInstance = OwnerChar->GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInstance))
	{
		EndHeavyAttack();
		return;
	}

	UAnimMontage* ReleaseMontage = WeaponData->GetHeavyAttackMontage();
	if (!IsValid(ReleaseMontage))
	{
		EndHeavyAttack();
		return;
	}

	// Tier is determined by whether the timer fired (auto) or the player released manually.
	// bIsAutoRelease is set by OnChargeAutoRelease before this function is called.
	ActiveTier     = bIsAutoRelease ? ERVHeavyAttackTier::AutoRelease : ERVHeavyAttackTier::Manual;
	bIsAutoRelease = false;

	// Transition state before stopping charge montage.
	// OnChargeMontageBlendingOut checks bIsHeavyCharging to detect external interruptions —
	// setting it false here tells that handler this stop is intentional.
	bIsHeavyCharging  = false;
	bIsHeavyAttacking = true;
	
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
	if (!bIsHeavyCharging && !bIsHeavyAttacking) { return; }

	// Stop whichever montage is currently active so visual and logical state stay in sync.
	// Required when called via ForceEndAllActions (Phase 3).
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (IsValid(OwnerChar))
	{
		UAnimInstance* AnimInstance = OwnerChar->GetMesh()->GetAnimInstance();
		if (IsValid(AnimInstance))
		{
			const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
			if (IsValid(WeaponData))
			{
				if (bIsHeavyCharging)
				{
					AnimInstance->Montage_Stop(0.1f, WeaponData->GetHeavyChargeMontage());
				}
				else
				{
					AnimInstance->Montage_Stop(0.1f, WeaponData->GetHeavyAttackMontage());
				}
			}
		}
	}

	bIsHeavyCharging  = false;
	bIsHeavyAttacking = false;
	bCanHeavyRelease  = false;
	bPendingRelease   = false;
	bIsAutoRelease    = false;
	GetWorld()->GetTimerManager().ClearTimer(ChargeAutoReleaseHandle);
	AttributeComponent->ResumeStaminaRegen();
}

void URVCombatComponent::OnChargeMontageBlendingOut(UAnimMontage* /*InMontage*/, bool /*bInterrupted*/)
{
	// bIsHeavyCharging still true = external interruption before release — clean up.
	// bIsHeavyCharging false = ExecuteHeavyAttack already transitioned us — ignore.
	if (bIsHeavyCharging)
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
	if (!CanPerformActionWith(ERVCombatState::Guarding)) { return; }
	if (!IsGrounded()) { return; }

	const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
	if (!IsValid(WeaponData) || !IsValid(WeaponData->GetDodgeMontage())) { return; }

	if (!AttributeComponent->ConsumeStamina(WeaponData->DodgeStaminaCost)) { return; }

	if (bIsSprinting) { EndSprint(); }
	if (bIsGuarding)  { EndGuard(); }

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!IsValid(OwnerChar)) { return; }

	UAnimInstance* AnimInstance = OwnerChar->GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInstance)) { return; }

	OwnerChar->SetActorRotation(InDodgeDirection.ToOrientationRotator());
	MovementComponent->bOrientRotationToMovement = false;

	bIsDodging = true;
	AttributeComponent->PauseStaminaRegen();

	UAnimMontage* DodgeMontage = WeaponData->GetDodgeMontage();

	FOnMontageBlendingOutStarted BlendOutDelegate;
	BlendOutDelegate.BindUObject(this, &URVCombatComponent::OnDodgeMontageBlendingOut);

	AnimInstance->Montage_Play(DodgeMontage);
	AnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, DodgeMontage);
}

void URVCombatComponent::SetDodgeIFrame(bool bActivate)
{
	if (!bIsDodging && bActivate) { return; }
	bIsInvincible = bActivate;
}

void URVCombatComponent::EndDodge()
{
	if (!bIsDodging) { return; }

	bIsDodging    = false;
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
	if (!CanPerformActionWith()) { return; }
	if (!IsGrounded()) { return; }
	if (bIsSprinting) { EndSprint(); }

	bIsGuarding = true;
	AttributeComponent->PauseStaminaRegen();
}

void URVCombatComponent::EndGuard()
{
	if (!bIsGuarding) { return; }

	bIsGuarding = false;
	AttributeComponent->ResumeStaminaRegen();
}

void URVCombatComponent::HandleGuardHit(float InDamageAmount)
{
	const bool bGuardHeld = AttributeComponent->ApplyStaminaDamage(InDamageAmount);
	if (!bGuardHeld) { return; }

	const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
	if (!IsValid(WeaponData) || !IsValid(WeaponData->GetGuardHitMontage())) { return; }

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!IsValid(OwnerChar)) { return; }

	UAnimInstance* AnimInstance = OwnerChar->GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInstance)) { return; }

	AnimInstance->Montage_Play(WeaponData->GetGuardHitMontage());
}

//--- Guard Break -------------------------------------------------------------

void URVCombatComponent::OnGuardBreakHandler()
{
	bIsGuarding    = false;
	bIsGuardBroken = true;
	AttributeComponent->PauseStaminaRegen();

	URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
	if (!IsValid(WeaponData) || !IsValid(WeaponData->GetGuardBreakMontage())) { return; }

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!IsValid(OwnerChar)) { return; }

	UAnimInstance* AnimInstance = OwnerChar->GetMesh()->GetAnimInstance();
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
	bIsGuardBroken = false;
	AttributeComponent->ResumeStaminaRegen();
}

//--- Sprint ------------------------------------------------------------------

void URVCombatComponent::StartSprint()
{
	if (bIsSprinting) { return; }
	if (!CanPerformActionWith()) { return; }
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