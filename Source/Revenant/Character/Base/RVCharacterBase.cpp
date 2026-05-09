#include "Character/Base/RVCharacterBase.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVComboComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVHeavyAttackComponent.h"
#include "Component/RVDodgeComponent.h"
#include "Component/RVGuardComponent.h"
#include "Component/RVSprintComponent.h"
#include "Data/RVCharacterDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"

ARVCharacterBase::ARVCharacterBase()
{
    PrimaryActorTick.bCanEverTick = false;

    AttributeComponent   = CreateDefaultSubobject<URVAttributeComponent>  (TEXT("AttributeComponent"));
    ComboComponent       = CreateDefaultSubobject<URVComboComponent>       (TEXT("ComboComponent"));
    EquipmentComponent   = CreateDefaultSubobject<URVEquipmentComponent>   (TEXT("EquipmentComponent"));
    CombatStateComponent = CreateDefaultSubobject<URVCombatStateComponent> (TEXT("CombatStateComponent"));
    HeavyAttackComponent = CreateDefaultSubobject<URVHeavyAttackComponent> (TEXT("HeavyAttackComponent"));
    DodgeComponent       = CreateDefaultSubobject<URVDodgeComponent>       (TEXT("DodgeComponent"));
    GuardComponent       = CreateDefaultSubobject<URVGuardComponent>       (TEXT("GuardComponent"));
    SprintComponent      = CreateDefaultSubobject<URVSprintComponent>      (TEXT("SprintComponent"));
}

void ARVCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    ensureMsgf(IsValid(AttributeComponent),   TEXT("[%s] AttributeComponent missing"),   *GetName());
    ensureMsgf(IsValid(ComboComponent),       TEXT("[%s] ComboComponent missing"),       *GetName());
    ensureMsgf(IsValid(EquipmentComponent),   TEXT("[%s] EquipmentComponent missing"),   *GetName());
    ensureMsgf(IsValid(CombatStateComponent), TEXT("[%s] CombatStateComponent missing"), *GetName());
    ensureMsgf(IsValid(HeavyAttackComponent), TEXT("[%s] HeavyAttackComponent missing"), *GetName());
    ensureMsgf(IsValid(DodgeComponent),       TEXT("[%s] DodgeComponent missing"),       *GetName());
    ensureMsgf(IsValid(GuardComponent),       TEXT("[%s] GuardComponent missing"),       *GetName());
    ensureMsgf(IsValid(SprintComponent),      TEXT("[%s] SprintComponent missing"),      *GetName());

    // --- Reference injection (Composition Root) ------------------------------

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();

    CombatStateComponent->InitReferences(this, EquipmentComponent, MoveComp);
    HeavyAttackComponent->InitReferences(this, CombatStateComponent, AttributeComponent, EquipmentComponent);
    DodgeComponent->InitReferences      (this, CombatStateComponent, AttributeComponent, EquipmentComponent);
    GuardComponent->InitReferences      (this, CombatStateComponent, AttributeComponent, EquipmentComponent);
    // SprintComponent finds its own dependencies in BeginPlay via FindComponentByClass.
    // OnStateChanged and OnForceEnd subscriptions are also wired in SprintComponent::BeginPlay.

    // --- Delegate wiring (Composition Root) ----------------------------------

    // Stamina depleted → GuardComponent interprets context (guard break or future use)
    AttributeComponent->OnStaminaDepleted.AddDynamic(GuardComponent, &URVGuardComponent::OnStaminaDepletedHandler);

    // Combo state → CombatStateComponent keeps Attacking bit in sync
    // Names reflect CombatStateComponent's perspective — it doesn't know about Combo
    ComboComponent->OnComboStarted.AddUObject(CombatStateComponent, &URVCombatStateComponent::OnAttackStarted);
    ComboComponent->OnComboEnded.AddUObject  (CombatStateComponent, &URVCombatStateComponent::OnAttackEnded);

    // ForceEnd → each action component self-cleans its own state
    CombatStateComponent->OnForceEnd.AddUObject(HeavyAttackComponent, &URVHeavyAttackComponent::ForceEndHeavyAttack);
    CombatStateComponent->OnForceEnd.AddUObject(DodgeComponent,       &URVDodgeComponent::ForceEndDodge);
    CombatStateComponent->OnForceEnd.AddUObject(GuardComponent,       &URVGuardComponent::ForceEndGuard);
    // SprintComponent and ComboComponent subscribe to OnForceEnd in their own BeginPlay

    // --- DataAsset init ------------------------------------------------------

    if (IsValid(CharacterData))
    {
        AttributeComponent->InitFromDataAsset(CharacterData);
    }
}

// --- IRVCombatInterface ------------------------------------------------------

void ARVCharacterBase::ActivateHitCheck()
{
    CombatStateComponent->PerformAttackTrace();
}

// --- IRVDamageable -----------------------------------------------------------

bool ARVCharacterBase::ApplyDamage(float InDamageAmount, AActor* InInstigator)
{
    // Dodge i-frame — all incoming damage blocked
    if (CombatStateComponent->IsInvincible()) { return false; }

    // Guarding — absorbed as stamina damage; may trigger guard break
    if (CombatStateComponent->IsInState(ERVCombatState::Guarding))
    {
        GuardComponent->HandleGuardHit(InDamageAmount);
        return true;
    }

    // Normal hit — route to HP
    return AttributeComponent->ApplyDamage(InInstigator, InDamageAmount);
}

void ARVCharacterBase::OnHitReaction(FVector InHitDirection)
{
}

// --- Movement ----------------------------------------------------------------

void ARVCharacterBase::Falling()
{
    Super::Falling();

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    OriginalRotationRate   = MoveComp->RotationRate;
    MoveComp->RotationRate = AirRotationRate;
}

void ARVCharacterBase::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);
    GetCharacterMovement()->RotationRate = OriginalRotationRate;
}