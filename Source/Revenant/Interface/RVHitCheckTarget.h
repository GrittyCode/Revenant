#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RVHitCheckTarget.generated.h"

UINTERFACE(MinimalAPI)
class URVHitCheckTarget : public UInterface
{
	GENERATED_BODY()
};

class REVENANT_API IRVHitCheckTarget
{
	GENERATED_BODY()

public:
	// Called by AnimNotify at the exact frame the weapon should deal damage.
	virtual void ActivateHitCheck() = 0;
};
