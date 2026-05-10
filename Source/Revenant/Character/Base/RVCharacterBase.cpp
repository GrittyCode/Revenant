#include "Character/Base/RVCharacterBase.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVComboComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVHeavyAttackComponent.h"
#include "Component/RVDodgeComponent.h"
#include "Component/RVGuardComponent.h"
#include "Component/RVSprintComponent.h"
#include "Component/RVHitreactioncomponent.h"
#include "Data/RVCharacterDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"

ARVCharacterBase::ARVCharacterBase()
{
    PrimaryActorTick.bCanEverTick = false;

    AttributeComponent    = CreateDefaultSubobject<URVAttributeComponent>   (TEXT("AttributeComponent"));
    ComboComponent        = CreateDefaultSubobject<URVComboComponent>        (TEXT("ComboComponent"));
    EquipmentComponent    = CreateDefaultSubobject<URVEquipmentComponent>    (TEXT("EquipmentComponent"));
    CombatStateComponent  = CreateDefaultSubobject<URVCombatStateComponent>  (TEXT("CombatStateComponent"));
    HeavyAttackComponent  = CreateDefaultSubobject<URVHeavyAttackComponent>  (TEXT("HeavyAttackComponent"));
    DodgeComponent        = CreateDefaultSubobject<URVDodgeComponent>        (TEXT("DodgeComponent"));
    GuardComponent        = CreateDefaultSubobject<URVGuardComponent>        (TEXT("GuardComponent"));
    SprintComponent       = CreateDefaultSubobject<URVSprintComponent>       (TEXT("SprintComponent"));
    HitReactionComponent  = CreateDefaultSubobject<URVHitReactionComponent>  (TEXT("HitReactionComponent"));
}

void ARVCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    ensureMsgf(IsValid(AttributeComponent),    TEXT("[%s] AttributeComponent missing"),    *GetName());
    ensureMsgf(IsValid(ComboComponent),        TEXT("[%s] ComboComponent missing"),        *GetName());
    ensureMsgf(IsValid(EquipmentComponent),    TEXT("[%s] EquipmentComponent missing"),    *GetName());
    ensureMsgf(IsValid(CombatStateComponent),  TEXT("[%s] CombatStateComponent missing"),  *GetName());
    ensureMsgf(IsValid(HeavyAttackComponent),  TEXT("[%s] HeavyAttackComponent missing"),  *GetName());
    ensureMsgf(IsValid(DodgeComponent),        TEXT("[%s] DodgeComponent missing"),        *GetName());
    ensureMsgf(IsValid(GuardComponent),        TEXT("[%s] GuardComponent missing"),        *GetName());
    ensureMsgf(IsValid(SprintComponent),       TEXT("[%s] SprintComponent missing"),       *GetName());
    ensureMsgf(IsValid(HitReactionComponent),  TEXT("[%s] HitReactionComponent missing"),  *GetName());

    // --- DataAsset init ------------------------------------------------------

    if (IsValid(CharacterData))
    {
        AttributeComponent->InitFromDataAsset(CharacterData);
    }

    // --- Reference injection (Composition Root) ------------------------------

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();

    CombatStateComponent->InitReferences(this, EquipmentComponent, MoveComp);
    HeavyAttackComponent->InitReferences(this, CombatStateComponent, AttributeComponent, EquipmentComponent);
    DodgeComponent->InitReferences      (this, CombatStateComponent, AttributeComponent, EquipmentComponent);
    GuardComponent->InitReferences      (this, CombatStateComponent, AttributeComponent, EquipmentComponent);
    HitReactionComponent->InitReferences(this, CombatStateComponent, AttributeComponent, CharacterData);
    // SprintComponent finds its own dependencies in BeginPlay via FindComponentByClass.

    // --- Delegate wiring (Composition Root) ----------------------------------

    // Stamina depleted → GuardComponent interprets context (guard break or future use)
    AttributeComponent->OnStaminaDepleted.AddDynamic(
        GuardComponent, &URVGuardComponent::OnStaminaDepletedHandler);

    // Combo state → CombatStateComponent keeps Attacking bit in sync
    ComboComponent->OnComboStarted.AddUObject(CombatStateComponent, &URVCombatStateComponent::OnAttackStarted);
    ComboComponent->OnComboEnded.AddUObject  (CombatStateComponent, &URVCombatStateComponent::OnAttackEnded);

    // ForceEnd → each action component self-cleans its own state
    // HitReactionComponent does NOT subscribe — its states (HitReaction/Groggy/Knockdown)
    // are managed exclusively by HitReactionComponent and are not cleared by ForceEnd.
    CombatStateComponent->OnForceEnd.AddUObject(HeavyAttackComponent, &URVHeavyAttackComponent::ForceEndHeavyAttack);
    CombatStateComponent->OnForceEnd.AddUObject(DodgeComponent,       &URVDodgeComponent::ForceEndDodge);
    CombatStateComponent->OnForceEnd.AddUObject(GuardComponent,       &URVGuardComponent::ForceEndGuard);
    // SprintComponent and ComboComponent subscribe to OnForceEnd in their own BeginPlay
}

// --- IRVCombatInterface ------------------------------------------------------

void ARVCharacterBase::ActivateHitCheck()
{
    CombatStateComponent->PerformAttackTrace();
}

// --- IRVDamageable -----------------------------------------------------------

bool ARVCharacterBase::ApplyDamage(const FRVHitInfo& InHitInfo)
{
    // Dodge i-frame — all incoming damage blocked
    if (CombatStateComponent->IsInvincible()) { return false; }

    // Guarding — absorbed as stamina damage; may trigger guard break via OnStaminaDepleted
    if (CombatStateComponent->IsInState(ERVCombatState::Guarding))
    {
        GuardComponent->HandleGuardHit(InHitInfo.Damage);
        return true;
    }

    // Normal hit — apply HP damage first, then handle poise / hit reaction.
    // HP and Poise are independent: character can take poise damage even at low HP.
    const bool bSurvived = AttributeComponent->ApplyDamage(InHitInfo.Instigator, InHitInfo.Damage);

    // Hit reaction runs regardless of survival — a dying character should still
    // play the stagger or knockdown animation before the death sequence.
    HitReactionComponent->HandleHit(InHitInfo);

    return bSurvived;
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