#include "Component/RVAttackTraceComponent.h"
#include "Data/RVMontageStatData.h"
#include "Data/RVAttackActionMultiplierRow.h"
#include "Interface/RVDamageable.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Components/MeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Particles/ParticleSystem.h"

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
    // TraceMesh is set by ARVCharacterBase::BeginPlay via InitTraceMesh()
    // after all components have self-initialized.
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

void URVAttackTraceComponent::OpenHitWindow()
{
    HitActors.Empty();
}

void URVAttackTraceComponent::CloseHitWindow()
{
    HitActors.Empty();
}

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
        TEXT("[%s] PerformAttackTrace: TraceMesh is null — assign via GetWeaponTraceMesh()"),
        *GetNameSafe(OwnerCharacter))) { return; }

    if (!ensureMsgf(
        TraceMesh->DoesSocketExist(FName("WeaponRoot")) &&
        TraceMesh->DoesSocketExist(FName("WeaponTip")),
        TEXT("[%s] PerformAttackTrace: WeaponRoot or WeaponTip socket missing"),
        *GetNameSafe(OwnerCharacter))) { return; }

    const FVector Root = TraceMesh->GetSocketLocation(FName("WeaponRoot"));
    const FVector Tip  = TraceMesh->GetSocketLocation(FName("WeaponTip"));

    const FVector Center     = (Root + Tip) * 0.5f;
    const float   HalfHeight = FVector::Dist(Root, Tip) * 0.5f;
    const FQuat   Rotation   = FRotationMatrix::MakeFromZ(Tip - Root).ToQuat();

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

        TWeakObjectPtr<AActor> WeakHitActor(HitActor);
        if (HitActors.Contains(WeakHitActor)) { continue; }
        HitActors.Add(WeakHitActor);

        if (IRVDamageable* Target = Cast<IRVDamageable>(HitActor))
        {
            FRVHitInfo HitInfo;
            HitInfo.Damage       = Damage;
            HitInfo.PoiseDamage  = PoiseDamage;
            HitInfo.Instigator   = OwnerCharacter;
            // Z zeroed before normalizing — vertical angle never influences knockback.
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
