#include "Character/Base/RVCharacterBase.h"
#include "Component/Attribute/RVVitalComponent.h"
#include "Component/Combat/RVHitReactionComponent.h"
#include "Component/Combat/RVAttackTraceComponent.h"
#include "Components/CapsuleComponent.h"
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

    VitalComponent       = CreateDefaultSubobject<URVVitalComponent>      (TEXT("VitalComponent"));
    CombatStateComponent = CreateDefaultSubobject<URVCombatStateComponent>(TEXT("CombatStateComponent"));
    HitReactionComponent = CreateDefaultSubobject<URVHitReactionComponent>(TEXT("HitReactionComponent"));
    AttackTraceComponent = CreateDefaultSubobject<URVAttackTraceComponent>(TEXT("AttackTraceComponent"));
}

void ARVCharacterBase::BeginPlay()
{
    Super::BeginPlay();
	
    OriginalRotationRate = GetCharacterMovement()->RotationRate;

    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

    AttackTraceComponent->InitTraceMesh(GetWeaponTraceMesh());

    InitStats();

    VitalComponent->OnDeath.AddDynamic(this, &ARVCharacterBase::OnDeath);

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

    const bool bSurvived = VitalComponent->ApplyDamage(InHitInfo.Instigator, InHitInfo.Damage);
    if (bSurvived) { HitReactionComponent->HandleHit(InHitInfo); }
    return bSurvived;
}

//--- Health queries ----------------------------------------------------------

float ARVCharacterBase::GetHealthRatio() const { return VitalComponent->GetHealthPercent(); }

FRVOnHealthChanged& ARVCharacterBase::GetOnHealthChanged() { return VitalComponent->OnHealthChanged; }
FRVOnDeath&         ARVCharacterBase::GetOnDeath()         { return VitalComponent->OnDeath; }
FRVOnPoiseDepleted& ARVCharacterBase::GetOnPoiseDepleted() { return VitalComponent->OnPoiseDepleted; }
FRVOnPoiseChanged&  ARVCharacterBase::GetOnPoiseChanged()  { return VitalComponent->OnPoiseChanged; }

//--- AnimNotify entry points -------------------------------------------------

void ARVCharacterBase::OpenAttackHitWindow()  { AttackTraceComponent->OpenHitWindow(); }
void ARVCharacterBase::CloseAttackHitWindow() { AttackTraceComponent->CloseHitWindow(); }

//--- AnimInstance state queries ----------------------------------------------

float ARVCharacterBase::GetStaggerDirection() const { return HitReactionComponent->GetStaggerDirection(); }

//--- Combat state operations -------------------------------------------------

bool ARVCharacterBase::CanAct(ERVCombatState InCoexistableStates) const
{
	return CombatStateComponent->CheckAvailableState(InCoexistableStates);
}

void ARVCharacterBase::AddCombatState(ERVCombatState InState)       { CombatStateComponent->AddState(InState); }
void ARVCharacterBase::RemoveCombatState(ERVCombatState InState)    { CombatStateComponent->RemoveState(InState); }
bool ARVCharacterBase::HasCombatState(ERVCombatState InState) const { return CombatStateComponent->HasState(InState); }


void ARVCharacterBase::SetInvincible(bool b) { CombatStateComponent->SetInvincible(b); }

bool ARVCharacterBase::IsInvincible()  const { return CombatStateComponent->IsInvincible(); }

void ARVCharacterBase::ForceEndAllActions()  { CombatStateComponent->ForceEndAllActions(); }

bool ARVCharacterBase::IsGrounded() const
{
	const UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	return IsValid(MoveComp) && !MoveComp->IsFalling();
}


//--- Poise operations --------------------------------------------------------

float ARVCharacterBase::GetMaxPoise()    const        { return VitalComponent->GetMaxPoise(); }
float ARVCharacterBase::GetPoiseRatio()  const        { return VitalComponent->GetPoiseRatio(); }
void  ARVCharacterBase::ApplyPoiseDamage(float InAmt) { VitalComponent->ApplyPoiseDamage(InAmt); }
void  ARVCharacterBase::ResetPoise()                  { VitalComponent->ResetPoise(); }

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