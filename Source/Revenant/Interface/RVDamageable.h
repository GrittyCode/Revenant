// Source/Revenant/Interface/RVDamageable.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RVDamageable.generated.h"

/**
 * Determines how URVHitReactionComponent resolves poise depletion.
 * Normal : standard hit — Stagger on poise depletion, Knockdown only if airborne or in HitReaction
 * Heavy  : heavy attack — Knockdown on poise depletion regardless of character state
 */
UENUM(BlueprintType)
enum class ERVHitType : uint8
{
	Normal = 0,
	Heavy  = 1,
	Grab   = 2,
	Smash  = 3,
};

/**
 * Carries all information about a single hit.
 */
USTRUCT(BlueprintType)
struct REVENANT_API FRVHitInfo
{
	GENERATED_BODY()

	/** HP damage to apply. */
	UPROPERTY(BlueprintReadWrite)
	float Damage = 0.f;

	/** Poise damage to apply. Poise depletion triggers hit reaction. */
	UPROPERTY(BlueprintReadWrite)
	float PoiseDamage = 0.f;

	/**
	 * Determines reaction severity when poise is depleted.
	 * Heavy forces Knockdown regardless of poise or character state.
	 */
	UPROPERTY(BlueprintReadWrite)
	ERVHitType HitType = ERVHitType::Normal;

	/**
	 * World-space direction from instigator toward target (normalized).
	 * Used by URVHitReactionComponent to select directional stagger montage
	 * and drive the ABP additive flinch layer.
	 */
	UPROPERTY(BlueprintReadWrite)
	FVector HitDirection = FVector::ZeroVector;

	/** Actor that initiated the attack. */
	UPROPERTY(BlueprintReadWrite)
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
	/* Returns true if the target survived (HP > 0 after hit).*/
	virtual bool ApplyDamage(const FRVHitInfo& InHitInfo) = 0;
};