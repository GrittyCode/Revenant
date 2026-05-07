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

    // URVCombatComponent must only be placed on ARVCharacterBase subclasses.
    ensureMsgf(IsValid(AttributeComponent), TEXT("[%s] URVAttributeComponent missing — CombatComponent requires ARVCharacterBase"), *GetNameSafe(Owner));
    ensureMsgf(IsValid(EquipmentComponent), TEXT("[%s] URVEquipmentComponent missing — CombatComponent requires ARVCharacterBase"), *GetNameSafe(Owner));
    ensureMsgf(IsValid(ComboComponent),     TEXT("[%s] URVComboComponent missing — CombatComponent requires ARVCharacterBase"),     *GetNameSafe(Owner));
    ensureMsgf(IsValid(MovementComponent),  TEXT("[%s] CharacterMovementComponent missing — owner must be ACharacter subclass"),    *GetNameSafe(Owner));

    AttributeComponent->OnGuardBreak.AddDynamic(this, &URVCombatComponent::OnGuardBreakHandler);

    // Non-dynamic delegate — ComboComponent has no knowledge of CombatComponent
    ComboComponent->OnComboStarted.AddUObject(this, &URVCombatComponent::OnComboStartedHandler);
    ComboComponent->OnComboEnded.AddUObject(this, &URVCombatComponent::OnComboEndedHandler);
}

// --- State -------------------------------------------------------------------

void URVCombatComponent::SetAttacking(bool bInIsAttacking)
{
    bIsAttacking = bInIsAttacking;
}

void URVCombatComponent::OnComboStartedHandler()
{
    SetAttacking(true);

    if (bIsGuarding)
    {
        bIsGuarding = false;
    }
	
	if (bIsSprinting)
	{
		EndSprint();
	}
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
    if (bIsAttacking)   { States |= ERVCombatState::Attacking; }
    if (bIsDodging)     { States |= ERVCombatState::Dodging; }
    if (bIsGuarding)    { States |= ERVCombatState::Guarding; }
    if (bIsGuardBroken) { States |= ERVCombatState::GuardBroken; }
    return States;
}

bool URVCombatComponent::CanPerformAction(ERVCombatState InAllowedActiveStates) const
{
    const ERVCombatState BlockingStates =
        ERVCombatState::Attacking   |
        ERVCombatState::Dodging     |
        ERVCombatState::Guarding    |
        ERVCombatState::GuardBroken;

    const ERVCombatState Relevant = (GetActiveStates() & BlockingStates) & ~InAllowedActiveStates;
    return Relevant == ERVCombatState::None;
}

// --- Attack Trace ------------------------------------------------------------

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
		Overlaps,
		Center,
		Rotation,
		ECC_Pawn,
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

		// One hit per actor per swing
		TWeakObjectPtr<AActor> WeakHitActor(HitActor);
		if (HitActors.Contains(WeakHitActor)) { continue; }
		HitActors.Add(WeakHitActor);

		if (IRVDamageable* Target = Cast<IRVDamageable>(HitActor))
		{
			Target->ApplyDamage(WeaponData->AttackDamage, OwnerChar);
		}
	}
}

// --- Combo -------------------------------------------------------------------

void URVCombatComponent::TryStartCombo()
{
    if (!ComboComponent->IsComboActive())
    {
        if (!IsGrounded()) { return; }

        // Attacking excluded — allows combo buffer input while already attacking
        // Guarding excluded — TryStartCombo is not called while guarding (InputAttack binds separately)
        if (!CanPerformAction(ERVCombatState::Attacking | ERVCombatState::Guarding)) { return; }
    }

    ComboComponent->HandleComboInput();
}

// --- Dodge -------------------------------------------------------------------

void URVCombatComponent::StartDodge(const FVector& InDodgeDirection)
{
    // Guard is excluded — dodge auto-cancels guard on entry
    if (!CanPerformAction(ERVCombatState::Guarding)) { return; }

    // Dodge is ground-only — airborne state blocks entry
    if (!IsGrounded()) { return; }

    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData) || !IsValid(WeaponData->GetDodgeMontage())) { return; }

    if (!AttributeComponent->ConsumeStamina(WeaponData->DodgeStaminaCost)) { return; }

    // Sprint and guard are interrupted by dodge
    if (bIsSprinting) { EndSprint(); }
    if (bIsGuarding)  { EndGuard(); }

    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!IsValid(OwnerChar)) { return; }

    UAnimInstance* AnimInstance = OwnerChar->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInstance)) { return; }

    // Rotate character to face dodge direction before playing Root Motion montage
    OwnerChar->SetActorRotation(InDodgeDirection.ToOrientationRotator());

    // Prevent movement component from fighting Root Motion rotation
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
    bIsDodging    = false;
    bIsInvincible = false;

    // Restore movement-driven rotation after roll completes
    MovementComponent->bOrientRotationToMovement = true;

    AttributeComponent->ResumeStaminaRegen();
}

void URVCombatComponent::OnDodgeMontageBlendingOut(UAnimMontage* /*InMontage*/, bool /*bInterrupted*/)
{
    EndDodge();
}

// --- Guard -------------------------------------------------------------------

void URVCombatComponent::StartGuard()
{
    // All blocking states checked — cannot enter guard mid-combo, mid-dodge, or while broken
    if (!CanPerformAction()) { return; }

    // Guard is ground-only — same principle as dodge
    if (!IsGrounded()) { return; }

    // Sprint is interrupted by guard
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
    // Apply stamina damage first — may fire OnGuardBreak if stamina reaches 0.
    // If guard broke, OnGuardBreakHandler runs synchronously and plays the break montage.
    // In that case we skip the hit reaction to avoid montage overlap.
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

// --- Guard Break -------------------------------------------------------------

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

// --- Sprint ------------------------------------------------------------------

void URVCombatComponent::StartSprint()
{
    if (bIsSprinting || bIsDodging || bIsGuarding || bIsGuardBroken || !IsGrounded()) { return; }
	
	if (AttributeComponent->GetCurrentStamina() <= 0.f) { return; }
	
    // Cache the BP-configured WalkSpeed so EndSprint restores the exact value
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