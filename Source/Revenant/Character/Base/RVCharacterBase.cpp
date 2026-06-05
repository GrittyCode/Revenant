#include "Character/Base/RVCharacterBase.h"
#include "Component/Attribute/RVVitalComponent.h"
#include "Component/Combat/RVHitReactionComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Data/Asset/RVMontageStatData.h"
#include "Data/Row/RVAttackActionMultiplierRow.h"
#include "Engine/OverlapResult.h"

static const FName SocketWeaponRoot(TEXT("WeaponRoot"));
static const FName SocketWeaponTip(TEXT("WeaponTip"));

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
}

void ARVCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    OriginalRotationRate = GetCharacterMovement()->RotationRate;

    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

    WeaponTraceMesh = GetWeaponTraceMesh();
    // DummyTarget legitimately has no attack sockets — only check null here.
    ensureMsgf(IsValid(WeaponTraceMesh),
        TEXT("[%s] GetWeaponTraceMesh() returned null"), *GetNameSafe(this));

    InitStats();

    //--- Wiring --------------------------------------------------------------

    VitalComponent->OnDeath.AddUObject(this, &ARVCharacterBase::OnDeath);
    CombatStateComponent->OnForceEnd.AddUObject(this, &ARVCharacterBase::CloseAttackHitWindow);
}

//--- IRVDamageable -----------------------------------------------------------

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

//--- Hit window --------------------------------------------------------------

void ARVCharacterBase::OpenAttackHitWindow()  { HitActors.Empty(); }
void ARVCharacterBase::CloseAttackHitWindow() { HitActors.Empty(); }

void ARVCharacterBase::ActivateHitCheck()
{
    if (!IsValid(WeaponTraceMesh))                           { return; }
    if (!WeaponTraceMesh->DoesSocketExist(SocketWeaponRoot)) { return; }
    if (!WeaponTraceMesh->DoesSocketExist(SocketWeaponTip))  { return; }

    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    if (!ensureMsgf(IsValid(AnimInst),
        TEXT("[%s] ActivateHitCheck: AnimInstance missing — check ABP assignment"),
        *GetNameSafe(this))) { return; }

    UAnimMontage* CurrentMontage = AnimInst->GetCurrentActiveMontage();
    const URVMontageStatData* StatData = CurrentMontage
        ? CurrentMontage->GetAssetUserData<URVMontageStatData>()
        : nullptr;
    const FRVAttackActionMultiplierRow* AttackStat = StatData ? StatData->GetStatRow() : nullptr;

    const float DmgMult   = AttackStat ? AttackStat->DamageMultiplier      : 1.f;
    const float PoiseMult = AttackStat ? AttackStat->PoiseDamageMultiplier : 1.f;
    const float Damage      = CachedBaseDamage      * DmgMult;
    const float PoiseDamage = CachedBasePoiseDamage * PoiseMult;

    const FVector Root = WeaponTraceMesh->GetSocketLocation(SocketWeaponRoot);
    const FVector Tip  = WeaponTraceMesh->GetSocketLocation(SocketWeaponTip);

    const float HalfHeight = FVector::Dist(Root, Tip) * 0.5f;
    if (HalfHeight < KINDA_SMALL_NUMBER) { return; }

    const FVector Center   = (Root + Tip) * 0.5f;
    const FQuat   Rotation = FRotationMatrix::MakeFromZ(Tip - Root).ToQuat();

    FCollisionObjectQueryParams ObjectQueryParams;
    ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    TArray<FOverlapResult> Overlaps;
    GetWorld()->OverlapMultiByObjectType(
        Overlaps, Center, Rotation, ObjectQueryParams,
        FCollisionShape::MakeCapsule(CachedAttackRadius, HalfHeight),
        QueryParams);

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* HitActor = Overlap.GetActor();
        if (!IsValid(HitActor))           { continue; }
        if (HitActors.Contains(HitActor)) { continue; }

        HitActors.Add(HitActor);

        IRVDamageable* Target = Cast<IRVDamageable>(HitActor);
        if (!Target) { continue; }

        FRVHitInfo HitInfo;
        HitInfo.Damage      = Damage;
        HitInfo.PoiseDamage = PoiseDamage;
        HitInfo.Instigator  = this;
        const FVector RawDir = GetActorLocation() - HitActor->GetActorLocation();
        HitInfo.HitDirection = FVector(RawDir.X, RawDir.Y, 0.f).GetSafeNormal();

        const bool bDamageApplied = Target->ApplyDamage(HitInfo);
        if (!bDamageApplied) { continue; }

        OnHitConfirmed.Broadcast(HitActor->GetActorLocation());
    }
}

//--- AnimInstance state queries ----------------------------------------------

float ARVCharacterBase::GetStaggerDirection() const { return HitReactionComponent->GetStaggerDirection(); }

//--- Combat state operations -------------------------------------------------

bool ARVCharacterBase::CanAct(ERVCombatState InCoexistableStates) const
{
    return CombatStateComponent->CheckAvailableState(InCoexistableStates);
}

void ARVCharacterBase::AddCombatState   (ERVCombatState InState)       { CombatStateComponent->AddState(InState); }
void ARVCharacterBase::RemoveCombatState(ERVCombatState InState)        { CombatStateComponent->RemoveState(InState); }
bool ARVCharacterBase::HasCombatState   (ERVCombatState InState) const  { return CombatStateComponent->HasState(InState); }

void ARVCharacterBase::SetInvincible(bool b) { CombatStateComponent->SetInvincible(b); }
bool ARVCharacterBase::IsInvincible()  const { return CombatStateComponent->IsInvincible(); }
void ARVCharacterBase::ForceEndAllActions()  { CombatStateComponent->ForceEndAllActions(); }

bool ARVCharacterBase::IsGrounded() const
{
    return !GetCharacterMovement()->IsFalling();
}

//--- Poise operations --------------------------------------------------------

float ARVCharacterBase::GetMaxPoise()    const        { return VitalComponent->GetMaxPoise(); }
float ARVCharacterBase::GetPoiseRatio()  const        { return VitalComponent->GetPoiseRatio(); }
void  ARVCharacterBase::ApplyPoiseDamage(float InAmt) { VitalComponent->ApplyPoiseDamage(InAmt); }
void  ARVCharacterBase::ResetPoise()                  { VitalComponent->ResetPoise(); }

//--- Attack stat injection ---------------------------------------------------

void ARVCharacterBase::SetCombatStat(float InDamage, float InPoise, float InRadius)
{
    CachedBaseDamage      = InDamage;
    CachedBasePoiseDamage = InPoise;
    CachedAttackRadius    = InRadius;
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
    GetCharacterMovement()->RotationRate = AirRotationRate;
}

void ARVCharacterBase::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);
    GetCharacterMovement()->RotationRate = OriginalRotationRate;
}
