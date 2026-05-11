#include "Character/Base/RVCharacterBase.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVComboComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVHeavyAttackComponent.h"
#include "Component/RVDodgeComponent.h"
#include "Component/RVGuardComponent.h"
#include "Component/RVSprintComponent.h"
#include "Component/RVHitReactionComponent.h"
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

    if (IsValid(CharacterData))
    {
        AttributeComponent->InitFromDataAsset(CharacterData);
    }

    //--- Reference Injection (Composition Root) ------------------------------

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();

    CombatStateComponent->InitReferences (this, EquipmentComponent, MoveComp);
    ComboComponent->InitReferences       (this, CombatStateComponent, AttributeComponent, EquipmentComponent);
    HeavyAttackComponent->InitReferences (this, CombatStateComponent, AttributeComponent, EquipmentComponent);
    DodgeComponent->InitReferences       (this, CombatStateComponent, AttributeComponent, EquipmentComponent);
    GuardComponent->InitReferences       (this, CombatStateComponent, AttributeComponent, EquipmentComponent);
    HitReactionComponent->InitReferences (this, CombatStateComponent, AttributeComponent, EquipmentComponent, CharacterData);

    //--- Delegate Wiring (Composition Root) ----------------------------------

    AttributeComponent->OnStaminaDepleted.AddDynamic(
        GuardComponent, &URVGuardComponent::OnStaminaDepletedHandler);

    GuardComponent->OnGuardBreakTriggered.AddUObject(
        HitReactionComponent, &URVHitReactionComponent::TriggerStaggerWithMontage);

    ComboComponent->OnComboStarted.AddUObject(CombatStateComponent, &URVCombatStateComponent::OnAttackStarted);
    ComboComponent->OnComboEnded.AddUObject  (CombatStateComponent, &URVCombatStateComponent::OnAttackEnded);

    CombatStateComponent->OnForceEnd.AddUObject(HeavyAttackComponent, &URVHeavyAttackComponent::ForceEndHeavyAttack);
    CombatStateComponent->OnForceEnd.AddUObject(DodgeComponent,       &URVDodgeComponent::ForceEndDodge);
    CombatStateComponent->OnForceEnd.AddUObject(GuardComponent,       &URVGuardComponent::ForceEndGuard);
    // ComboComponent and SprintComponent subscribe to OnForceEnd inside their own InitReferences/BeginPlay.
}

//--- IRVCombatInterface ------------------------------------------------------

void ARVCharacterBase::ActivateHitCheck()
{
    CombatStateComponent->PerformAttackTrace();
}

//--- IRVDamageable -----------------------------------------------------------

bool ARVCharacterBase::ApplyDamage(const FRVHitInfo& InHitInfo)
{
    if (CombatStateComponent->IsInvincible()) { return false; }

    if (CombatStateComponent->IsInState(ERVCombatState::Guarding))
    {
        GuardComponent->HandleGuardHit(InHitInfo.Damage);
        return true;
    }

    const bool bSurvived = AttributeComponent->ApplyDamage(InHitInfo.Instigator, InHitInfo.Damage);
    HitReactionComponent->HandleHit(InHitInfo);

    return bSurvived;
}

//--- Movement ----------------------------------------------------------------

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