#include "Component/Combat/RVAttackTraceComponent.h"
#include "Data/Asset/RVMontageStatData.h"
#include "Data/Row/RVAttackActionMultiplierRow.h"
#include "Interface/RVDamageable.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Components/MeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Particles/ParticleSystem.h"

static const FName SocketWeaponRoot(TEXT("WeaponRoot"));
static const FName SocketWeaponTip(TEXT("WeaponTip"));

URVAttackTraceComponent::URVAttackTraceComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URVAttackTraceComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerCharacter = Cast<ACharacter>(GetOwner());
    ensureMsgf(IsValid(OwnerCharacter),
        TEXT("[URVAttackTraceComponent] Owner must be ACharacter"));
}

void URVAttackTraceComponent::InitTraceMesh(UMeshComponent* InTraceMesh)
{
    TraceMesh = InTraceMesh;
    ensureMsgf(IsValid(TraceMesh),
        TEXT("[%s] URVAttackTraceComponent: TraceMesh is null — GetWeaponTraceMesh() returned null"),
        *GetNameSafe(OwnerCharacter));
}

void URVAttackTraceComponent::SetCombatStat(
    float InBaseDamage, float InBasePoiseDamage, float InAttackRadius)
{
    CachedBaseDamage      = InBaseDamage;
    CachedBasePoiseDamage = InBasePoiseDamage;
    CachedAttackRadius    = InAttackRadius;
}

void URVAttackTraceComponent::SetHitFX(
    UNiagaraSystem* InNiagara, UParticleSystem* InCascade, USoundBase* InSFX)
{
    HitImpactEffect        = InNiagara;
    HitImpactEffectCascade = InCascade;
    HitSFX                 = InSFX;
}

void URVAttackTraceComponent::OpenHitWindow()  { HitActors.Empty(); }
void URVAttackTraceComponent::CloseHitWindow() { HitActors.Empty(); }

void URVAttackTraceComponent::PerformAttackTrace()
{
    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    UAnimMontage* CurrentMontage = AnimInst->GetCurrentActiveMontage();

    const URVMontageStatData* StatData = CurrentMontage
        ? CurrentMontage->GetAssetUserData<URVMontageStatData>()
        : nullptr;

    const FRVAttackActionMultiplierRow* AttackStat = StatData ? StatData->GetStatRow() : nullptr;

    const float DmgMult   = AttackStat ? AttackStat->DamageMultiplier      : 1.f;
    const float PoiseMult = AttackStat ? AttackStat->PoiseDamageMultiplier : 1.f;
    const float Damage      = CachedBaseDamage      * DmgMult;
    const float PoiseDamage = CachedBasePoiseDamage * PoiseMult;

    if (!ensureMsgf(IsValid(TraceMesh),
        TEXT("[%s] PerformAttackTrace: TraceMesh is null"),
        *GetNameSafe(OwnerCharacter))) { return; }

    if (!ensureMsgf(
        TraceMesh->DoesSocketExist(SocketWeaponRoot) &&
        TraceMesh->DoesSocketExist(SocketWeaponTip),
        TEXT("[%s] PerformAttackTrace: WeaponRoot or WeaponTip socket missing"),
        *GetNameSafe(OwnerCharacter))) { return; }

    const FVector Root = TraceMesh->GetSocketLocation(SocketWeaponRoot);
    const FVector Tip  = TraceMesh->GetSocketLocation(SocketWeaponTip);

    const float HalfHeight = FVector::Dist(Root, Tip) * 0.5f;
    if (HalfHeight < KINDA_SMALL_NUMBER) { return; }

    const FVector Center   = (Root + Tip) * 0.5f;
    const FQuat   Rotation = FRotationMatrix::MakeFromZ(Tip - Root).ToQuat();

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    TArray<FOverlapResult> Overlaps;
    GetWorld()->OverlapMultiByChannel(
        Overlaps, Center, Rotation, ECC_Pawn,
        FCollisionShape::MakeCapsule(CachedAttackRadius, HalfHeight),
        Params);

#if !UE_BUILD_SHIPPING
    DrawDebugCapsule(GetWorld(), Center, HalfHeight, CachedAttackRadius,
                     Rotation, FColor::Red, false, 1.f);
#endif

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* HitActor = Overlap.GetActor();
        if (!IsValid(HitActor)) { continue; }
        if (HitActors.Contains(HitActor)) { continue; }

        HitActors.Add(HitActor);

        if (IRVDamageable* Target = Cast<IRVDamageable>(HitActor))
        {
            FRVHitInfo HitInfo;
            HitInfo.Damage       = Damage;
            HitInfo.PoiseDamage  = PoiseDamage;
            HitInfo.Instigator   = OwnerCharacter;
            const FVector RawDir = OwnerCharacter->GetActorLocation() - HitActor->GetActorLocation();
            HitInfo.HitDirection = FVector(RawDir.X, RawDir.Y, 0.f).GetSafeNormal();

            Target->ApplyDamage(HitInfo);

            const FVector ImpactLocation = HitActor->GetActorLocation();

            if (IsValid(HitImpactEffect))
            {
                UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                    GetWorld(), HitImpactEffect, ImpactLocation);
            }
            else if (IsValid(HitImpactEffectCascade))
            {
                UGameplayStatics::SpawnEmitterAtLocation(
                    GetWorld(), HitImpactEffectCascade, ImpactLocation);
            }

            if (IsValid(HitSFX))
            {
                UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSFX, ImpactLocation);
            }
        }
    }
}