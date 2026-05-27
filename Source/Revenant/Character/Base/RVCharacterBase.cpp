#include "Character/Base/RVCharacterBase.h"
#include "Component/RVHitReactionComponent.h"
#include "Component/RVAttackTraceComponent.h"
#include "Components/CapsuleComponent.h"
#include "Data/RVCharacterDataAsset.h"
#include "Data/RVCharacterStatRow.h"
#include "GameFramework/CharacterMovementComponent.h"

ARVCharacterBase::ARVCharacterBase()
{
    PrimaryActorTick.bCanEverTick = false;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    MoveComp->bOrientRotationToMovement = true;
    MoveComp->RotationRate = FRotator(0.f, 500.f, 0.f);

    GetCapsuleComponent()->CanCharacterStepUpOn = ECB_No;

    AttributeComponent    = CreateDefaultSubobject<URVAttributeComponent>   (TEXT("AttributeComponent"));
    CombatStateComponent  = CreateDefaultSubobject<URVCombatStateComponent> (TEXT("CombatStateComponent"));
    HitReactionComponent  = CreateDefaultSubobject<URVHitReactionComponent> (TEXT("HitReactionComponent"));
    AttackTraceComponent  = CreateDefaultSubobject<URVAttackTraceComponent> (TEXT("AttackTraceComponent"));
}

void ARVCharacterBase::BeginPlay()
{
    Super::BeginPlay(); // Dispatches BeginPlay to all components first.

    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

    // Components are created via CreateDefaultSubobject — null here is an engine-level failure.
    ensureMsgf(IsValid(AttributeComponent),   TEXT("[%s] AttributeComponent missing"),   *GetName());
    ensureMsgf(IsValid(CombatStateComponent), TEXT("[%s] CombatStateComponent missing"), *GetName());
    ensureMsgf(IsValid(HitReactionComponent), TEXT("[%s] HitReactionComponent missing"), *GetName());
    ensureMsgf(IsValid(AttackTraceComponent), TEXT("[%s] AttackTraceComponent missing"), *GetName());

    // TraceMesh supplied by CharacterBase because only CharacterBase knows GetWeaponTraceMesh().
    // All four components have already self-initialized their Owner references in their own BeginPlay.
    AttackTraceComponent->InitTraceMesh(GetWeaponTraceMesh());

    InitStats();

    // HitReactionComponent self-initializes Owner via GetOwner().
    // Float params are data-driven from CharacterData — owned by Base, passed here.
    const FRVCharacterStatRow* StatRow        = IsValid(CharacterData) ? CharacterData->GetStatRow() : nullptr;
    const float StaggerDuration    = StatRow ? StatRow->StaggerDuration    : 0.5f;
    const float StaggerThreshold   = StatRow ? StatRow->StaggerThreshold   : 0.5f;
    const float KnockdownThreshold = StatRow ? StatRow->KnockdownThreshold : 0.4f;

    HitReactionComponent->InitParams(
        GetHitReactionAnimData(), StaggerDuration, StaggerThreshold, KnockdownThreshold);

    //--- Base-level delegate wiring (CharacterBase owns both sides) ----------

    // Death — actor subscribes to its own component.
    AttributeComponent->OnDeath.AddDynamic(this, &ARVCharacterBase::OnDeath);

    // ForceEnd → close hit window so interrupted attacks don't ghost-hit.
    CombatStateComponent->OnForceEnd.AddUObject(
        AttackTraceComponent, &URVAttackTraceComponent::CloseHitWindow);
}

//--- IRVHitCheckTarget / IRVDamageable ---------------------------------------

void ARVCharacterBase::ActivateHitCheck()
{
    AttackTraceComponent->PerformAttackTrace();
}

bool ARVCharacterBase::ApplyDamage(const FRVHitInfo& InHitInfo)
{
    if (CombatStateComponent->IsInvincible()) { return false; }

    const bool bSurvived = AttributeComponent->ApplyDamage(InHitInfo.Instigator, InHitInfo.Damage);

    if (bSurvived) { HitReactionComponent->HandleHit(InHitInfo); }

    return bSurvived;
}

//--- Attribute queries -------------------------------------------------------

float ARVCharacterBase::GetHealthRatio()  const { return AttributeComponent->GetHealthPercent(); }
float ARVCharacterBase::GetStaminaRatio() const { return AttributeComponent->GetStaminaPercent(); }

//--- Attribute event facades -------------------------------------------------

FRVOnHealthChanged&  ARVCharacterBase::GetOnHealthChanged()  { return AttributeComponent->OnHealthChanged; }
FRVOnStaminaChanged& ARVCharacterBase::GetOnStaminaChanged() { return AttributeComponent->OnStaminaChanged; }
FRVOnDeath&          ARVCharacterBase::GetOnDeath()          { return AttributeComponent->OnDeath; }
FRVOnPoiseDepleted&  ARVCharacterBase::GetOnPoiseDepleted()  { return AttributeComponent->OnPoiseDepleted; }
FRVOnPoiseChanged&   ARVCharacterBase::GetOnPoiseChanged()   { return AttributeComponent->OnPoiseChanged; }

//--- AnimNotify entry points -------------------------------------------------

void ARVCharacterBase::OpenAttackHitWindow()  { AttackTraceComponent->OpenHitWindow(); }
void ARVCharacterBase::CloseAttackHitWindow() { AttackTraceComponent->CloseHitWindow(); }

//--- AnimInstance state queries ----------------------------------------------

bool  ARVCharacterBase::IsInCombatState(ERVCombatState InState) const { return CombatStateComponent->IsInState(InState); }
float ARVCharacterBase::GetStaggerDirection() const                    { return HitReactionComponent->GetStaggerDirection(); }

//--- Combat state operations -------------------------------------------------

void ARVCharacterBase::AddCombatState(ERVCombatState InState)    { CombatStateComponent->AddState(InState); }
void ARVCharacterBase::RemoveCombatState(ERVCombatState InState) { CombatStateComponent->RemoveState(InState); }
bool ARVCharacterBase::HasCombatState(ERVCombatState InState) const { return CombatStateComponent->HasState(InState); }

bool ARVCharacterBase::CanAct(ERVCombatState InCoexistableStates) const
{
    return CombatStateComponent->CheckAvailableState(InCoexistableStates);
}

bool ARVCharacterBase::IsGrounded()  const { return CombatStateComponent->IsGrounded(); }

void ARVCharacterBase::SetInvincible(bool bInvincible) { CombatStateComponent->SetInvincible(bInvincible); }
bool ARVCharacterBase::IsInvincible() const            { return CombatStateComponent->IsInvincible(); }
void ARVCharacterBase::ForceEndAllActions()             { CombatStateComponent->ForceEndAllActions(); }

//--- Stamina operations ------------------------------------------------------

bool  ARVCharacterBase::TryConsumeStamina(float InAmount)  { return AttributeComponent->ConsumeStamina(InAmount); }
float ARVCharacterBase::GetCurrentStamina() const          { return AttributeComponent->GetCurrentStamina(); }
void  ARVCharacterBase::PauseStaminaRegen()                { AttributeComponent->PauseStaminaRegen(); }
void  ARVCharacterBase::ResumeStaminaRegen()               { AttributeComponent->ResumeStaminaRegen(); }
void  ARVCharacterBase::ResetStaminaRegenDelay()           { AttributeComponent->ResetStaminaRegenDelay(); }
bool  ARVCharacterBase::ApplyStaminaDamage(float InAmount) { return AttributeComponent->ApplyStaminaDamage(InAmount); }

//--- Poise operations --------------------------------------------------------

float ARVCharacterBase::GetMaxPoise()   const          { return AttributeComponent->GetMaxPoise(); }
float ARVCharacterBase::GetPoiseRatio() const          { return AttributeComponent->GetPoiseRatio(); }
bool  ARVCharacterBase::ApplyPoiseDamage(float InAmt)  { return AttributeComponent->ApplyPoiseDamage(InAmt); }
void  ARVCharacterBase::ResetPoise()                   { AttributeComponent->ResetPoise(); }

//--- Attack trace operations -------------------------------------------------

void ARVCharacterBase::SetCombatStat(float InDamage, float InPoise, float InRadius)
{
    AttackTraceComponent->SetCombatStat(InDamage, InPoise, InRadius);
}

void ARVCharacterBase::SetHitFX(UNiagaraSystem* InNiagara, UParticleSystem* InCascade, USoundBase* InSFX)
{
    AttackTraceComponent->SetHitFX(InNiagara, InCascade, InSFX);
}

//--- Hit reaction operations -------------------------------------------------

void ARVCharacterBase::TriggerStaggerWithMontage(UAnimMontage* InMontage)
{
    HitReactionComponent->TriggerStaggerWithMontage(InMontage);
}

//--- Lifecycle ---------------------------------------------------------------

void ARVCharacterBase::OnDeath() {}

FVector ARVCharacterBase::GetForwardLocation(float InOffset) const
{
    return GetActorLocation() + GetActorForwardVector() * InOffset;
}

FVector ARVCharacterBase::GetGroundOrigin() const
{
    return GetActorLocation() - FVector(0.f, 0.f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
}

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
