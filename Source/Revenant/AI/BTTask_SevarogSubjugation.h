#pragma once

#include "CoreMinimal.h"
#include "AI/BTTask_SevarogAttackBase.h"
#include "BTTask_SevarogSubjugation.generated.h"

UCLASS()
class REVENANT_API UBTTask_SevarogSubjugation : public UBTTask_SevarogAttackBase
{
	GENERATED_BODY()

public:
	UBTTask_SevarogSubjugation();

protected:
	virtual bool LaunchAttack(ARVSevarogCharacter* InBoss) override;
};
