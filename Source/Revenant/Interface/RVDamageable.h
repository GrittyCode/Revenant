// Source/Revenant/Interface/RVDamageable.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RVDamageable.generated.h"

/**
 * Carries all information about a single hit.
 */
USTRUCT(BlueprintType)
struct REVENANT_API FRVHitInfo
{
    GENERATED_BODY()

    float Damage = 0.f;
    float PoiseDamage = 0.f;

    /**
     * If true, forces Knockdown regardless of poise value or airborne state.
     */
    bool bForceKnockdown = false;

    /**
     * World-space direction from instigator toward target (normalized).

     */
    FVector HitDirection = FVector::ZeroVector;

    /** Actor that initiated the attack. */
    UPROPERTY()
    TObjectPtr<AActor> Instigator;
};

UINTERFACE(MinimalAPI)
class URVDamageable : public UInterface
{
    GENERATED_BODY()
};

class REVENANT_API IRVDamageable
{
    GENERATED_BODY()

public:
    /**
     * Applies damage from a single hit.
     * Returns true if the target survived (HP > 0 after hit).
     */
    virtual bool ApplyDamage(const FRVHitInfo& InHitInfo) = 0;
};