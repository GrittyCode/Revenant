#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RVDamageable.generated.h"

USTRUCT()
struct REVENANT_API FRVHitInfo
{
	GENERATED_BODY()

	UPROPERTY()
	float Damage = 0.f;

	UPROPERTY()
	float PoiseDamage = 0.f;

	// World-space direction from instigator toward target (normalized).
	UPROPERTY()
	FVector HitDirection = FVector::ZeroVector;

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
	virtual bool ApplyDamage(const FRVHitInfo& InHitInfo) = 0;
};