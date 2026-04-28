// Source/Revenant/Component/RVCombatComponent.cpp
#include "Component/RVCombatComponent.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Data/RVWeaponDataAsset.h"
#include "Interface/RVDamageable.h"
#include "GameFramework/Character.h"
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

	if (IsValid(AttributeComponent))
	{
		AttributeComponent->OnGuardBreak.AddDynamic(this, &URVCombatComponent::OnGuardBreakHandler);
	}
}

// --- State -------------------------------------------------------------------

void URVCombatComponent::SetAttacking(bool bInIsAttacking)
{
	bIsAttacking = bInIsAttacking;
}

ERVCombatState URVCombatComponent::GetActiveStates() const
{
	ERVCombatState States = ERVCombatState::None;
	if (bIsAttacking) { States |= ERVCombatState::Attacking; }
	if (bIsDodging) { States |= ERVCombatState::Dodging; }
	if (bIsGuarding) { States |= ERVCombatState::Guarding; }
	if (bIsGuardBroken) { States |= ERVCombatState::GuardBroken; }
	return States;
}

bool URVCombatComponent::CanPerformAction(ERVCombatState InAllowedActiveStates) const
{
	const ERVCombatState BlockingStates =
		ERVCombatState::Attacking |
		ERVCombatState::Dodging |
		ERVCombatState::Guarding |
		ERVCombatState::GuardBroken;

	const ERVCombatState Relevant = (GetActiveStates() & BlockingStates) & ~InAllowedActiveStates;
	return Relevant == ERVCombatState::None;
}


// --- Attack Trace ------------------------------------------------------------

void URVCombatComponent::PerformAttackTrace()
{
	if (!IsValid(EquipmentComponent)) { return; }

	URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
	if (!IsValid(WeaponData)) { return; }

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!IsValid(OwnerChar)) { return; }

	USkeletalMeshComponent* Mesh = OwnerChar->GetMesh();
	const FVector Root = Mesh->GetSocketLocation(FName("WeaponRoot"));
	const FVector Tip = Mesh->GetSocketLocation(FName("WeaponTip"));

	const FVector Center = (Root + Tip) * 0.5f;
	const float HalfHeight = FVector::Dist(Root, Tip) * 0.5f;
	const FQuat Rotation = FRotationMatrix::MakeFromZ(Tip - Root).ToQuat();

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerChar);

	TArray<FOverlapResult> Overlaps;
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

	TSet<AActor*> HitActors;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!IsValid(HitActor) || HitActors.Contains(HitActor)) { continue; }

		HitActors.Add(HitActor);

		if (IRVDamageable* Target = Cast<IRVDamageable>(HitActor))
		{
			Target->ApplyDamage(WeaponData->AttackDamage, OwnerChar);
		}
	}
}

// --- Dodge -------------------------------------------------------------------

void URVCombatComponent::StartDodge(const FVector& InDodgeDirection)
{
	// Guard is excluded -- dodge auto-cancels guard on entry
	if (!CanPerformAction(ERVCombatState::Guarding)) { return; }
	if (!IsValid(AttributeComponent) || !IsValid(EquipmentComponent)) { return; }

	URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
	if (!IsValid(WeaponData)) { return; }

	if (!AttributeComponent->ConsumeStamina(WeaponData->DodgeStaminaCost)) { return; }

	if (bIsGuarding)
	{
		EndGuard();
	}

	const int32 MontageIndex = GetDodgeMontageIndex(InDodgeDirection);
	if (!WeaponData->DodgeMontages.IsValidIndex(MontageIndex)) { return; }

	UAnimMontage* DodgeMontage = WeaponData->DodgeMontages[MontageIndex];
	if (!IsValid(DodgeMontage)) { return; }

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!IsValid(OwnerChar)) { return; }

	UAnimInstance* AnimInst = OwnerChar->GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInst)) { return; }

	bIsDodging = true;
	AttributeComponent->PauseStaminaRegen();

	FOnMontageBlendingOutStarted BlendOutDelegate;
	BlendOutDelegate.BindUObject(this, &URVCombatComponent::OnDodgeMontageBlendingOut);

	AnimInst->Montage_Play(DodgeMontage);
	AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, DodgeMontage);
}

void URVCombatComponent::SetDodgeIFrame(bool bActivate)
{
	if (!bIsDodging && bActivate) { return; }
	bIsInvincible = bActivate;
}

void URVCombatComponent::EndDodge()
{
	bIsDodging = false;
	bIsInvincible = false;

	if (IsValid(AttributeComponent))
	{
		AttributeComponent->ResumeStaminaRegen();
	}
}

void URVCombatComponent::OnDodgeMontageBlendingOut(UAnimMontage* /*InMontage*/, bool /*bInterrupted*/)
{
	EndDodge();
}

// --- Guard -------------------------------------------------------------------

void URVCombatComponent::StartGuard()
{
	// All blocking states checked -- cannot enter guard mid-combo, mid-dodge, or while broken
	if (!CanPerformAction()) { return; }
	if (!IsValid(AttributeComponent) || !IsValid(EquipmentComponent)) { return; }

	URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
	if (!IsValid(WeaponData) || !IsValid(WeaponData->GuardMontage)) { return; }

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!IsValid(OwnerChar)) { return; }

	bIsGuarding = true;
	AttributeComponent->PauseStaminaRegen();

	OwnerChar->GetMesh()->GetAnimInstance()->Montage_Play(WeaponData->GuardMontage);
}

void URVCombatComponent::EndGuard()
{
	if (!bIsGuarding) { return; }

	bIsGuarding = false;

	if (!IsValid(EquipmentComponent)) { return; }
	URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
	if (!IsValid(WeaponData) || !IsValid(WeaponData->GuardMontage)) { return; }

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!IsValid(OwnerChar)) { return; }

	UAnimInstance* AnimInst = OwnerChar->GetMesh()->GetAnimInstance();
	AnimInst->Montage_JumpToSection(FName("End"), WeaponData->GuardMontage);

	if (IsValid(AttributeComponent))
	{
		AttributeComponent->ResumeStaminaRegen();
	}
}

// --- Guard Break -------------------------------------------------------------

void URVCombatComponent::OnGuardBreakHandler()
{
	bIsGuarding = false;
	bIsGuardBroken = true;

	if (IsValid(AttributeComponent))
	{
		AttributeComponent->PauseStaminaRegen();
	}

	if (!IsValid(EquipmentComponent)) { return; }
	URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
	if (!IsValid(WeaponData) || !IsValid(WeaponData->GuardBreakMontage)) { return; }

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!IsValid(OwnerChar)) { return; }

	OwnerChar->GetMesh()->GetAnimInstance()->Montage_Play(WeaponData->GuardBreakMontage);

	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		World->GetTimerManager().SetTimer(
			GuardBreakRecoveryHandle,
			this,
			&URVCombatComponent::OnGuardBreakRecoveryComplete,
			GuardBreakRecoveryTime,
			false
		);
	}
}

void URVCombatComponent::OnGuardBreakRecoveryComplete()
{
	bIsGuardBroken = false;

	if (IsValid(AttributeComponent))
	{
		AttributeComponent->ResumeStaminaRegen();
	}
}

// --- Helpers -----------------------------------------------------------------

int32 URVCombatComponent::GetDodgeMontageIndex(const FVector& InDodgeDirection) const
{
	if (InDodgeDirection.IsNearlyZero()) { return 0; }

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!IsValid(OwnerChar)) { return 0; }

	const FVector LocalDir = OwnerChar->GetActorTransform().InverseTransformVectorNoScale(InDodgeDirection);
	const FVector NormDir = LocalDir.GetSafeNormal();

	const float DotF = FVector::DotProduct(NormDir, FVector::ForwardVector);
	const float DotR = FVector::DotProduct(NormDir, FVector::RightVector);

	if (FMath::Abs(DotF) >= FMath::Abs(DotR))
	{
		return DotF >= 0.f ? 0 : 1;
	}
	else
	{
		return DotR >= 0.f ? 3 : 2;
	}
}
