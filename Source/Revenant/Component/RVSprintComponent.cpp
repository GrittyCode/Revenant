#include "Component/RVSprintComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVAttributeComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

URVSprintComponent::URVSprintComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URVSprintComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    CombatStateComponent = Owner->FindComponentByClass<URVCombatStateComponent>();
    AttributeComponent   = Owner->FindComponentByClass<URVAttributeComponent>();

    ACharacter* OwnerChar = Cast<ACharacter>(Owner);
    if (IsValid(OwnerChar))
    {
        MovementComponent = OwnerChar->GetCharacterMovement();
    }

    ensureMsgf(IsValid(CombatStateComponent), TEXT("[%s] URVCombatStateComponent missing — SprintComponent requires ARVCharacterBase"), *GetNameSafe(Owner));
    ensureMsgf(IsValid(AttributeComponent),   TEXT("[%s] URVAttributeComponent missing — SprintComponent requires ARVCharacterBase"),   *GetNameSafe(Owner));
    ensureMsgf(IsValid(MovementComponent),    TEXT("[%s] CharacterMovementComponent missing — SprintComponent requires ACharacter"),     *GetNameSafe(Owner));

    // Self-terminate when any blocking combat state becomes active.
    // This removes the need for HeavyAttack / Dodge / Guard / Combo to call EndSprint.
    CombatStateComponent->OnStateChanged.AddUObject(this, &URVSprintComponent::OnCombatStateChanged);

    // Self-terminate on force-end (hit reaction Phase 3).
    CombatStateComponent->OnForceEnd.AddUObject(this, &URVSprintComponent::ForceEndSprint);
}

void URVSprintComponent::StartSprint()
{
    if (bIsSprinting) { return; }
    if (!CombatStateComponent->CheckAvailableState()) { return; }
    if (!CombatStateComponent->IsGrounded()) { return; }
    if (AttributeComponent->GetCurrentStamina() <= 0.f) { return; }

    OriginalWalkSpeed = MovementComponent->MaxWalkSpeed;
    MovementComponent->MaxWalkSpeed = SprintSpeed;
    bIsSprinting = true;
}

void URVSprintComponent::EndSprint()
{
    if (!bIsSprinting) { return; }

    bIsSprinting = false;
    MovementComponent->MaxWalkSpeed = OriginalWalkSpeed;
}

void URVSprintComponent::ForceEndSprint()
{
    EndSprint();
}

void URVSprintComponent::OnCombatStateChanged(ERVCombatState InNewState)
{
    if (!bIsSprinting) { return; }

    // End sprint when any blocking state becomes active.
    // CheckAvailableState with no coexistable states = any blocking state present → false.
    if (!CombatStateComponent->CheckAvailableState())
    {
        EndSprint();
    }
}