#include "AI/BTTask_SevarogRush.h"
#include "AI/RVAIController.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "Data/Asset/RVSevarogDataAsset.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_SevarogRush::UBTTask_SevarogRush()
{
    NodeName = TEXT("Sevarog Rush");
}

bool UBTTask_SevarogRush::LaunchAttack(ARVSevarogCharacter* InBoss)
{
    return InBoss->ExecuteRushAttack();
}

EBTNodeResult::Type UBTTask_SevarogRush::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    bAttackLaunched = false;

    ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
    if (!IsValid(Controller)) { return EBTNodeResult::Failed; }

    ARVSevarogCharacter* Boss   = Controller->GetBossCharacter();
    APawn*               Player = Controller->GetPlayerPawn();
    if (!IsValid(Boss) || !IsValid(Player)) { return EBTNodeResult::Failed; }

    Boss->StartRush();

    FAIMoveRequest MoveReq(Player);
    MoveReq.SetAcceptanceRadius(Boss->GetSevarogData()->ArrivalRange * 0.5f);
    MoveReq.SetCanStrafe(false);

    const FPathFollowingRequestResult MoveResult = Controller->MoveTo(MoveReq);

    if (MoveResult.Code == EPathFollowingRequestResult::Failed)
    {
        Boss->EndRush();
        return EBTNodeResult::Failed;
    }

    // AlreadyAtGoal falls through — TickTask handles EndRush + rotation + launch.
    return EBTNodeResult::InProgress;
}

void UBTTask_SevarogRush::TickTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    if (bAttackLaunched) { return; }

    ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
    if (!IsValid(Controller)) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

    ARVSevarogCharacter* Boss   = Controller->GetBossCharacter();
    APawn*               Player = Controller->GetPlayerPawn();
    if (!IsValid(Boss) || !IsValid(Player)) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

    if (Boss->IsRushing())
    {
        // Still in movement phase — wait for arrival.
        const float Dist = FVector::Dist(Boss->GetActorLocation(), Player->GetActorLocation());
        if (Dist > Boss->GetSevarogData()->ArrivalRange) { return; }

        Controller->StopMovement();
        Boss->EndRush();
    }

    // Arrived — align before attacking.
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

EBTNodeResult::Type UBTTask_SevarogRush::AbortTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::AbortTask(OwnerComp, NodeMemory);

    ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
    if (IsValid(Controller)) { Controller->StopMovement(); }

    return EBTNodeResult::Aborted;
}

void UBTTask_SevarogRush::OnTaskFinished(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

    ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
    if (!IsValid(Controller)) { return; }

    ARVSevarogCharacter* Boss = Controller->GetBossCharacter();
    if (!IsValid(Boss)) { return; }

    if (Boss->IsRushing()) { Boss->EndRush(); }
}