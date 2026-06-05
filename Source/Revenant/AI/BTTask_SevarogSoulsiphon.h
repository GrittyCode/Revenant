#pragma once

#include "CoreMinimal.h"
#include "AI/BTTask_SevarogRotationAttackBase.h"
#include "BTTask_SevarogSoulsiphon.generated.h"

UCLASS()
class REVENANT_API UBTTask_SevarogSoulsiphon : public UBTTask_SevarogRotationAttackBase
{
	GENERATED_BODY()

public:
	UBTTask_SevarogSoulsiphon();

protected:
	virtual bool LaunchAttack(ARVSevarogCharacter* InBoss) override;
};