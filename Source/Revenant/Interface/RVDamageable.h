#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RVDamageable.generated.h"

USTRUCT(BlueprintType)
struct REVENANT_API FRVHitInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	float Damage = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float PoiseDamage = 0.f;

	// World-space direction from instigator toward target (normalized).
	UPROPERTY(BlueprintReadWrite)
	FVector HitDirection = FVector::ZeroVector;

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
	virtual bool ApplyDamage(const FRVHitInfo& InHitInfo) = 0;
};