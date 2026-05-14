#include "Component/RVSprintComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVAttributeComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

URVSprintComponent::URVSprintComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URVSprintComponent::InitReferences(
    ACharacter* InOwnerCharacter,
    URVCombatStateComponent* InCombatStateComponent,
    URVAttributeComponent* InAttributeComponent)
{
    ensureMsgf(IsValid(InOwnerCharacter),       TEXT("[%s] InOwnerCharacter is null"),       *GetNameSafe(GetOwner()));
    ensureMsgf(IsValid(InCombatStateComponent), TEXT("[%s] InCombatStateComponent is null"), *GetNameSafe(GetOwner()));
    ensureMsgf(IsValid(InAttributeComponent),   TEXT("[%s] InAttributeComponent is null"),   *GetNameSafe(GetOwner()));

    CombatStateComponent = InCombatStateComponent;
    AttributeComponent   = InAttributeComponent;
    MovementComponent    = InOwnerCharacter->GetCharacterMovement();
    CombatStateComponent->OnStateChanged.AddUObject(this, &URVSprintComponent::OnCombatStateChanged);
    CombatStateComponent->OnForceEnd.AddUObject(this, &URVSprintComponent::ForceEndSprint);
}

void URVSprintComponent::BeginPlay()
{
    Super::BeginPlay();
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

    if (!CombatStateComponent->CheckAvailableState())
    {
        EndSprint();
    }
}