#include "Character/Enemy/RVDummyTarget.h"
#include "Interface/RVDamageable.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

ARVDummyTarget::ARVDummyTarget()
{
    PrimaryActorTick.bCanEverTick = true;

    GetMesh()->SetRelativeLocationAndRotation(
        FVector(0.f, 0.f, -88.f),
        FRotator(0.f, -90.f, 0.f)
    );
}

void ARVDummyTarget::BeginPlay()
{
    Super::BeginPlay();

    TimeUntilNextDamage = DealDamageInterval;

    if (bDealPeriodicDamage)
    {
        CachedPlayer = UGameplayStatics::GetPlayerPawn(this, 0);
        ensureMsgf(CachedPlayer.IsValid(),
            TEXT("[ARVDummyTarget] Player pawn not found — place a Player Start in the level"));
    }
}

void ARVDummyTarget::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // --- Periodic damage tick ------------------------------------------------

    if (bDealPeriodicDamage)
    {
        TimeUntilNextDamage -= DeltaTime;

        if (TimeUntilNextDamage <= 0.f)
        {
            DealDamageToPlayer();
            TimeUntilNextDamage = DealDamageInterval;
        }
    }

    // --- Hit display timer ---------------------------------------------------

    if (HitDisplayTimer > 0.f)
    {
        HitDisplayTimer -= DeltaTime;
    }

    // --- Debug label (stripped in Shipping) ----------------------------------

#if !UE_BUILD_SHIPPING
    const FVector LabelLocation = GetActorLocation() + FVector(0.f, 0.f, 120.f);

    if (HitDisplayTimer > 0.f)
    {
        DrawDebugString(GetWorld(), LabelLocation,
            FString::Printf(TEXT("HIT! -%.0f"), LastReceivedDamage),
            nullptr, FColor::Red, 0.f, true, 1.5f);
    }
    else if (bDealPeriodicDamage)
    {
        DrawDebugString(GetWorld(), LabelLocation,
            FString::Printf(TEXT("HIT in %.1fs"), FMath::Max(TimeUntilNextDamage, 0.f)),
            nullptr, FColor::Yellow, 0.f, true, 1.5f);
    }
    else
    {
        DrawDebugString(GetWorld(), LabelLocation,
            TEXT("DUMMY TARGET"),
            nullptr, FColor::White, 0.f, true, 1.2f);
    }
#endif
}

// --- IRVDamageable -----------------------------------------------------------

bool ARVDummyTarget::ApplyDamage(const FRVHitInfo& InHitInfo)
{
    // Route through ARVCharacterBase — handles i-frame / guard / HP reduction
    // and triggers URVHitReactionComponent for stagger / groggy / knockdown.
    const bool bResult = Super::ApplyDamage(InHitInfo);

    if (bResult)
    {
        LastReceivedDamage = InHitInfo.Damage;
        HitDisplayTimer    = HitDisplayDuration;

        UE_LOG(LogTemp, Warning, TEXT("[DummyTarget] %.0f dmg / %.0f poise from %s"),
            InHitInfo.Damage, InHitInfo.PoiseDamage, *GetNameSafe(InHitInfo.Instigator));

#if !UE_BUILD_SHIPPING
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange,
                FString::Printf(TEXT("[DummyTarget] %.0f dmg received"), InHitInfo.Damage));
        }
#endif
    }

    return bResult;
}

// --- Internal ----------------------------------------------------------------

void ARVDummyTarget::DealDamageToPlayer()
{
	if (!CachedPlayer.IsValid()) { return; }

	IRVDamageable* Target = Cast<IRVDamageable>(CachedPlayer.Get());
	if (!Target) { return; }

	FRVHitInfo HitInfo;
	HitInfo.Damage       = DealDamageAmount;
	HitInfo.PoiseDamage  = DealPoiseDamage;
	HitInfo.HitDirection = (GetActorLocation() - CachedPlayer->GetActorLocation()).GetSafeNormal();
	HitInfo.Instigator   = this;

	Target->ApplyDamage(HitInfo);

	UE_LOG(LogTemp, Warning, TEXT("[DummyTarget] Dealt %.0f dmg / %.0f poise to player"),
		DealDamageAmount, DealPoiseDamage);
}