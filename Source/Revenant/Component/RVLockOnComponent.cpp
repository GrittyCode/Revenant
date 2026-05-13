#include "Component/RVLockOnComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Interface/RVDamageable.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

URVLockOnComponent::URVLockOnComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    // Tick is meaningless when not locked on — disable until ToggleLockOn activates it.
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

//--- Reference Injection -----------------------------------------------------

void URVLockOnComponent::InitReferences(ACharacter* InOwnerCharacter,
                                         APlayerController* InPlayerController,
                                         URVCombatStateComponent* InCombatStateComponent)
{
    OwnerCharacter       = InOwnerCharacter;
    PlayerController     = InPlayerController;
    CombatStateComponent = InCombatStateComponent;
}

//--- Public API --------------------------------------------------------------

void URVLockOnComponent::ToggleLockOn()
{
    if (bIsLockedOn)
    {
        BreakLockOn();
        return;
    }

    AActor* Target = TryFindTarget();
    if (!IsValid(Target)) { return; }

    LockOnTarget = Target;
    bIsLockedOn  = true;

    // manual rotation takes over from here — prevent MovementComponent from fighting it
    OwnerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;

    SetComponentTickEnabled(true);
}

void URVLockOnComponent::BreakLockOn()
{
    bIsLockedOn  = false;
    LockOnTarget = nullptr;

    // EndDodge also sets this true — no conflict, both want the same result
    OwnerCharacter->GetCharacterMovement()->bOrientRotationToMovement = true;

    SetComponentTickEnabled(false);
}

AActor* URVLockOnComponent::GetLockOnTarget() const
{
    return LockOnTarget.IsValid() ? LockOnTarget.Get() : nullptr;
}

//--- Tick --------------------------------------------------------------------

void URVLockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsLockedOn) { return; }

    if (!LockOnTarget.IsValid())
    {
        BreakLockOn();
        return;
    }

    const float DistSq = FVector::DistSquared(OwnerCharacter->GetActorLocation(),
                                               LockOnTarget->GetActorLocation());
    if (DistSq > FMath::Square(AutoBreakRange))
    {
        BreakLockOn();
        return;
    }

    // EndDodge sets this true — re-suppress next frame
    OwnerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;

    UpdateCamera(DeltaTime);
    UpdateCharacterRotation(DeltaTime);
}

//--- Private -----------------------------------------------------------------

AActor* URVLockOnComponent::TryFindTarget() const
{
    TArray<AActor*> Candidates;
    UGameplayStatics::GetAllActorsWithInterface(GetWorld(), URVDamageable::StaticClass(), Candidates);

    const float HalfAngleRad = FMath::DegreesToRadians(LockOnSearchHalfAngle);

    FVector CameraLocation;
    FRotator CameraRotation;
    PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
    const FVector CameraForward = CameraRotation.Vector();

    AActor* BestTarget  = nullptr;
    float   BestDistSq  = FMath::Square(LockOnRange);

    for (AActor* Candidate : Candidates)
    {
        if (!IsValid(Candidate))            { continue; }
        if (Candidate == OwnerCharacter)    { continue; }

        const FVector ToCandidate = Candidate->GetActorLocation() - CameraLocation;
        const float   DistSq      = ToCandidate.SizeSquared();
        if (DistSq > FMath::Square(LockOnRange)) { continue; }

        const float Dot = FVector::DotProduct(CameraForward, ToCandidate.GetSafeNormal());
        if (Dot < FMath::Cos(HalfAngleRad)) { continue; }

        if (DistSq < BestDistSq)
        {
            BestDistSq = DistSq;
            BestTarget = Candidate;
        }
    }

    return BestTarget;
}

void URVLockOnComponent::UpdateCamera(float DeltaTime) const
{
    FVector  CameraLocation;
    FRotator CameraRotation;
    PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

    const FVector  ToTarget   = LockOnTarget->GetActorLocation() - CameraLocation;
    const FRotator TargetRot  = ToTarget.ToOrientationRotator();
    const FRotator CurrentRot = PlayerController->GetControlRotation();

    PlayerController->SetControlRotation(
        FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, CameraInterpSpeed));
}

void URVLockOnComponent::UpdateCharacterRotation(float DeltaTime) const
{
    // root motion montages own rotation in these states
    if (CombatStateComponent->IsInState(
        ERVCombatState::Dodging | ERVCombatState::HitReaction | ERVCombatState::Knockdown))
    {
        return;
    }

    FVector ToTarget = LockOnTarget->GetActorLocation() - OwnerCharacter->GetActorLocation();
    ToTarget.Z = 0.f;
    if (ToTarget.IsNearlyZero()) { return; }

    const FRotator TargetRot  = ToTarget.ToOrientationRotator();
    const FRotator CurrentRot = OwnerCharacter->GetActorRotation();

    OwnerCharacter->SetActorRotation(
        FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, CharacterRotationInterpSpeed));
}