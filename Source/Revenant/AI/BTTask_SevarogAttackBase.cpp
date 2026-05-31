#include "AI/BTTask_SevarogAttackBase.h"
#include "AI/RVAIController.h"
#include "Character/Enemy/RVSevarogCharacter.h"

UBTTask_SevarogAttackBase::UBTTask_SevarogAttackBase()
{
	// bCreateNodeInstance ensures each execution gets its own instance,
	// making bAttackLaunched safe as a plain member variable.
	bCreateNodeInstance = true;
	bNotifyTick         = true;
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_SevarogAttackBase::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(Controller)) { return EBTNodeResult::Failed; }

	ARVSevarogCharacter* Boss = Controller->GetBossCharacter();
	if (!IsValid(Boss)) { return EBTNodeResult::Failed; }

	bAttackLaunched = false;

	// TickTask handles rotation and deferred LaunchAttack.
	return EBTNodeResult::InProgress;
}

void UBTTask_SevarogAttackBase::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (bAttackLaunched) { return; }

	ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(Controller)) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	ARVSevarogCharacter* Boss = Controller->GetBossCharacter();
	if (!IsValid(Boss)) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	if (!RotateBossTowardPlayer(Boss, DeltaSeconds)) { return; }

	// Aligned — launch the attack.
	bAttackLaunched = true;

	if (!LaunchAttack(Boss))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	SubscribeAttackFinished(OwnerComp, Boss);
}

bool UBTTask_SevarogAttackBase::RotateBossTowardPlayer(ARVSevarogCharacter* InBoss, float DeltaSeconds) const
{
	const ARVAIController* AICtrl = Cast<ARVAIController>(InBoss->GetController());

	if (!IsValid(AICtrl)) { return false; }

	const APawn* Player = AICtrl->GetPlayerPawn();
	if (!IsValid(Player)) { return false; }

	const FVector ToPlayer = (Player->GetActorLocation() - InBoss->GetActorLocation()).GetSafeNormal2D();
	if (ToPlayer.IsNearlyZero()) { return true; }

	const FRotator CurrentRot = InBoss->GetActorRotation();
	const FRotator TargetRot  = ToPlayer.Rotation();
	const float    DeltaYaw   = FMath::FindDeltaAngleDegrees(CurrentRot.Yaw, TargetRot.Yaw);

	if (FMath::Abs(DeltaYaw) <= AlignThresholdDeg) { return true; }

	const FRotator NewRot = FMath::RInterpConstantTo(CurrentRot, TargetRot, DeltaSeconds, TurnSpeed);
	InBoss->SetActorRotation(FRotator(0.f, NewRot.Yaw, 0.f));

	// Recheck after applying rotation.
	const float RemainingDelta = FMath::FindDeltaAngleDegrees(NewRot.Yaw, TargetRot.Yaw);
	return FMath::Abs(RemainingDelta) <= AlignThresholdDeg;
}

void UBTTask_SevarogAttackBase::SubscribeAttackFinished(
	UBehaviorTreeComponent& OwnerComp, ARVSevarogCharacter* InBoss)
{
	TWeakObjectPtr<UBehaviorTreeComponent> BTWeak(&OwnerComp);
	TWeakObjectPtr<ARVSevarogCharacter>    BossWeak(InBoss);

	InBoss->OnAttackFinished.AddWeakLambda(this, [BTWeak, BossWeak, this]()
	{
		if (BossWeak.IsValid()) { BossWeak->OnAttackFinished.RemoveAll(this); }
		if (BTWeak.IsValid())   { FinishLatentTask(*BTWeak.Get(), EBTNodeResult::Succeeded); }
	});
}

void UBTTask_SevarogAttackBase::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(Controller)) { return; }

	ARVSevarogCharacter* Boss = Controller->GetBossCharacter();
	if (!IsValid(Boss)) { return; }

	Boss->OnAttackFinished.RemoveAll(this);

	if (Boss->IsAttacking()) { Boss->ForceEndCurrentAction(); }
}