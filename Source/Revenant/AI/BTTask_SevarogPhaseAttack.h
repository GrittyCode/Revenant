#pragma once

#include "CoreMinimal.h"
#include "AI/BTTask_SevarogAttackBase.h"
#include "BTTask_SevarogPhaseAttack.generated.h"

UCLASS()
class REVENANT_API UBTTask_SevarogPhaseAttack : public UBTTask_SevarogAttackBase
{
	GENERATED_BODY()

public:
	UBTTask_SevarogPhaseAttack();

protected:
	virtual bool LaunchAttack(ARVSevarogCharacter* InBoss) override;
};
