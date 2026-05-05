#include "Component/RVCombatComponent.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Data/RVWeaponDataAsset.h"
#include "Interface/RVDamageable.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/CharacterMovementComponent.h"

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
    if (bIsAttacking)   { States |= ERVCombatState::Attacking; }
    if (bIsDodging)     { States |= ERVCombatState::Dodging; }
    if (bIsGuarding)    { States |= ERVCombatState::Guarding; }
    if (bIsGuardBroken) { States |= ERVCombatState::GuardBroken; }
    return States;
}

bool URVCombatComponent::CanPerformAction(ERVCombatState InAllowedActiveStates) const
{
    const ERVCombatState BlockingStates =
        ERVCombatState::Attacking |
        ERVCombatState::Dodging   |
        ERVCombatState::Guarding  |
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
    const FVector Tip  = Mesh->GetSocketLocation(FName("WeaponTip"));

    const FVector Center     = (Root + Tip) * 0.5f;
    const float   HalfHeight = FVector::Dist(Root, Tip) * 0.5f;
    const FQuat   Rotation   = FRotationMatrix::MakeFromZ(Tip - Root).ToQuat();

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

    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData) || !IsValid(WeaponData->GetDodgeMontage())) { return; }

    if (!AttributeComponent->ConsumeStamina(WeaponData->DodgeStaminaCost)) { return; }

    if (bIsGuarding)
    {
        EndGuard();
    }

    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!IsValid(OwnerChar)) { return; }

    UAnimInstance* AnimInstance = OwnerChar->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInstance)) { return; }

    // Rotate character to face dodge direction before playing Root Motion montage
    OwnerChar->SetActorRotation(InDodgeDirection.ToOrientationRotator());

    // Prevent movement component from fighting Root Motion rotation
    OwnerChar->GetCharacterMovement()->bOrientRotationToMovement = false;

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

    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (IsValid(OwnerChar))
    {
        // Restore movement-driven rotation after roll completes
        OwnerChar->GetCharacterMovement()->bOrientRotationToMovement = true;
    }

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
    if (!IsValid(AttributeComponent)) { return; }

    bIsGuarding = true;
    AttributeComponent->PauseStaminaRegen();

    // ABP will transition to BS_Guard_Locomotion branch via bIsGuarding flag
}

void URVCombatComponent::EndGuard()
{
    if (!bIsGuarding) { return; }

    bIsGuarding = false;

    if (IsValid(AttributeComponent))
    {
        AttributeComponent->ResumeStaminaRegen();
    }

    // ABP Blend Alpha interpolation handles the transition back to normal locomotion
}

// --- Guard Break -------------------------------------------------------------

void URVCombatComponent::OnGuardBreakHandler()
{
    bIsGuarding    = false;
    bIsGuardBroken = true;

    if (IsValid(AttributeComponent))
    {
        AttributeComponent->PauseStaminaRegen();
    }

    if (!IsValid(EquipmentComponent)) { return; }

    URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData) || !IsValid(WeaponData->GetGuardBreakMontage())) { return; }

    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!IsValid(OwnerChar)) { return; }

    OwnerChar->GetMesh()->GetAnimInstance()->Montage_Play(WeaponData->GetGuardBreakMontage());

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