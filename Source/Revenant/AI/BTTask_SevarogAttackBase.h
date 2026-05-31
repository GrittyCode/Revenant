#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SevarogAttackBase.generated.h"

class ARVSevarogCharacter;

UCLASS(Abstract)
class REVENANT_API UBTTask_SevarogAttackBase : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SevarogAttackBase();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

	/** Returns false if the boss cannot execute the action (groggy, already attacking, unassigned montage). */
	virtual bool LaunchAttack(ARVSevarogCharacter* InBoss) PURE_VIRTUAL(UBTTask_SevarogAttackBase::LaunchAttack, return false;);

	// Rotation speed (degrees/second) while aligning to face the player before attacking.
	UPROPERTY(EditAnywhere, Category = "RV|Alignment", meta = (ClampMin = "1.0"))
	float TurnSpeed = 480.f;

	// Attack launches once the yaw delta to the player is within this threshold (degrees).
	UPROPERTY(EditAnywhere, Category = "RV|Alignment", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float AlignThresholdDeg = 30.f;

	// Safe as a member because bCreateNodeInstance = true.
	bool bAttackLaunched = false;

	// Subscribes OnAttackFinished and calls FinishLatentTask on completion.
	void SubscribeAttackFinished(UBehaviorTreeComponent& OwnerComp, ARVSevarogCharacter* InBoss);

private:
	// Returns true when the boss is within AlignThresholdDeg of the player.
	bool RotateBossTowardPlayer(ARVSevarogCharacter* InBoss, float DeltaSeconds) const;
};