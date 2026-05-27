#include "Component/RVLockOnComponent.h"
#include "Character/Base/RVCharacterBase.h"
#include "Interface/RVDamageable.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

URVLockOnComponent::URVLockOnComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void URVLockOnComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerBase      = Cast<ARVCharacterBase>(GetOwner());
    OwnerCharacter = Cast<ACharacter>(GetOwner());

    ensureMsgf(IsValid(OwnerBase),
        TEXT("[URVLockOnComponent] Owner must be ARVCharacterBase"));
    ensureMsgf(IsValid(OwnerCharacter),
        TEXT("[URVLockOnComponent] Owner must be ACharacter"));

    PlayerController = Cast<APlayerController>(OwnerCharacter->GetController());
    ensureMsgf(IsValid(PlayerController),
        TEXT("[%s] URVLockOnComponent: PlayerController missing — ensure possession before BeginPlay"),
        *GetNameSafe(OwnerBase));
}

void URVLockOnComponent::ToggleLockOn()
{
    if (bIsLockedOn) { BreakLockOn(); return; }

    AActor* Target = TryFindTarget();
    if (!IsValid(Target)) { return; }

    LockOnTarget = Target;
    bIsLockedOn  = true;

    OwnerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
    SetComponentTickEnabled(true);
}

void URVLockOnComponent::BreakLockOn()
{
    bIsLockedOn  = false;
    LockOnTarget = nullptr;

    OwnerCharacter->GetCharacterMovement()->bOrientRotationToMovement = true;
    SetComponentTickEnabled(false);
}

AActor* URVLockOnComponent::GetLockOnTarget() const
{
    return LockOnTarget.IsValid() ? LockOnTarget.Get() : nullptr;
}

void URVLockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsLockedOn) { return; }
    if (!LockOnTarget.IsValid()) { BreakLockOn(); return; }

    const float DistSq = FVector::DistSquared(
        OwnerCharacter->GetActorLocation(), LockOnTarget->GetActorLocation());
    if (DistSq > FMath::Square(AutoBreakRange)) { BreakLockOn(); return; }

    // Re-suppress each tick — EndDodge restores bOrientRotationToMovement to true.
    OwnerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;

    UpdateCamera(DeltaTime);
    UpdateCharacterRotation(DeltaTime);
}

AActor* URVLockOnComponent::TryFindTarget() const
{
    TArray<AActor*> Candidates;
    UGameplayStatics::GetAllActorsWithInterface(
        GetWorld(), URVDamageable::StaticClass(), Candidates);

    const float HalfAngleRad = FMath::DegreesToRadians(LockOnSearchHalfAngle);

    FVector  CameraLocation;
    FRotator CameraRotation;
    PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
    const FVector CameraForward = CameraRotation.Vector();

    AActor* BestTarget = nullptr;
    float   BestDistSq = FMath::Square(LockOnRange);

    for (AActor* Candidate : Candidates)
    {
        if (!IsValid(Candidate))         { continue; }
        if (Candidate == OwnerCharacter) { continue; }

        const FVector ToCandidate = Candidate->GetActorLocation() - CameraLocation;
        const float   DistSq      = ToCandidate.SizeSquared();
        if (DistSq > FMath::Square(LockOnRange)) { continue; }

        const float Dot = FVector::DotProduct(CameraForward, ToCandidate.GetSafeNormal());
        if (Dot < FMath::Cos(HalfAngleRad)) { continue; }

        if (DistSq < BestDistSq) { BestDistSq = DistSq; BestTarget = Candidate; }
    }

    return BestTarget;
}

void URVLockOnComponent::UpdateCamera(float DeltaTime) const
{
    FVector  CameraLocation;
    FRotator CameraRotation;
    PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

    const FVector  ToTarget  = LockOnTarget->GetActorLocation() - CameraLocation;
    const FRotator TargetRot = ToTarget.ToOrientationRotator();

    PlayerController->SetControlRotation(
        FMath::RInterpTo(PlayerController->GetControlRotation(), TargetRot, DeltaTime, CameraInterpSpeed));
}

void URVLockOnComponent::UpdateCharacterRotation(float DeltaTime) const
{
    // Root motion montages own rotation during these states.
    if (OwnerBase->HasCombatState(
        ERVCombatState::Dodging | ERVCombatState::HitReaction | ERVCombatState::Knockdown))
    {
        return;
    }

    FVector ToTarget = LockOnTarget->GetActorLocation() - OwnerCharacter->GetActorLocation();
    ToTarget.Z = 0.f;
    if (ToTarget.IsNearlyZero()) { return; }

    OwnerCharacter->SetActorRotation(FMath::RInterpTo(
        OwnerCharacter->GetActorRotation(),
        ToTarget.ToOrientationRotator(),
        DeltaTime, CharacterRotationInterpSpeed));
}
