// Source/Revenant/Interface/RVDamageable.h  ← 파일명도 변경
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RVDamageable.generated.h"

UINTERFACE(MinimalAPI)
class URVDamageable : public UInterface
{
	GENERATED_BODY()
};

class REVENANT_API IRVDamageable
{
	GENERATED_BODY()

public:
	virtual bool ApplyDamage(float InDamageAmount, AActor* InInstigator) = 0;
	virtual void OnHitReaction(FVector InHitDirection) = 0;
};