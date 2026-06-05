#include "AI/BTTask_SevarogRotationAttackBase.h"
#include "AI/RVAIController.h"
#include "Character/Enemy/RVSevarogCharacter.h"

UBTTask_SevarogRotationAttackBase::UBTTask_SevarogRotationAttackBase()
{
    bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_SevarogRotationAttackBase::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
    if (!IsValid(Controller)) { return EBTNodeResult::Failed; }

    ARVSevarogCharacter* Boss = Controller->GetBossCharacter();
    if (!IsValid(Boss)) { return EBTNodeResult::Failed; }

    bAttackLaunched = false;

    return EBTNodeResult::InProgress;
}

void UBTTask_SevarogRotationAttackBase::TickTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    if (bAttackLaunched) { return; }

    ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
    if (!IsValid(Controller)) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

    ARVSevarogCharacter* Boss = Controller->GetBossCharacter();
    if (!IsValid(Boss)) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

    if (!RotateBossTowardPlayer(Boss, DeltaSeconds)) { return; }

    SubscribeAttackFinished(OwnerComp, Boss);

    if (!LaunchAttack(Boss))
    {
        Boss->OnAttackFinished.RemoveAll(this);
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    bAttackLaunched = true;
}

bool UBTTask_SevarogRotationAttackBase::RotateBossTowardPlayer(
    ARVSevarogCharacter* InBoss, float DeltaSeconds) const
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

    const float RemainingDelta = FMath::FindDeltaAngleDegrees(NewRot.Yaw, TargetRot.Yaw);
    return FMath::Abs(RemainingDelta) <= AlignThresholdDeg;
}