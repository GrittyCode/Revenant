#pragma once

#include "CoreMinimal.h"
#include "AI/BTTask_SevarogAttackBase.h"
#include "BTTask_SevarogRotationAttackBase.generated.h"

class ARVSevarogCharacter;

UCLASS(Abstract)
class REVENANT_API UBTTask_SevarogRotationAttackBase : public UBTTask_SevarogAttackBase
{
	GENERATED_BODY()

public:
	UBTTask_SevarogRotationAttackBase();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// Returns true when the boss yaw delta to the player is within AlignThresholdDeg.
	bool RotateBossTowardPlayer(ARVSevarogCharacter* InBoss, float DeltaSeconds) const;

	UPROPERTY(EditAnywhere, Category = "RV|Alignment", meta = (ClampMin = "1.0"))
	float TurnSpeed = 480.f;

	UPROPERTY(EditAnywhere, Category = "RV|Alignment", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float AlignThresholdDeg = 30.f;

	// Prevents re-entry after LaunchAttack is called from TickTask.
	bool bAttackLaunched = false;
};