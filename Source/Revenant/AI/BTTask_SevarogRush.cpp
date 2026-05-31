#include "AI/BTTask_SevarogRush.h"
#include "AI/RVAIController.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "Data/Asset/RVSevarogDataAsset.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_SevarogRush::UBTTask_SevarogRush()
{
    NodeName = TEXT("Sevarog Rush");
    // bCreateNodeInstance, bNotifyTick, bNotifyTaskFinished are set by the base constructor.
}

bool UBTTask_SevarogRush::LaunchAttack(ARVSevarogCharacter* InBoss)
{
    return InBoss->ExecuteRushAttack();
}

EBTNodeResult::Type UBTTask_SevarogRush::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    if (MoveResult.Code == EPathFollowingRequestResult::AlreadyAtGoal)
    {
        Boss->EndRush();

        if (!LaunchAttack(Boss)) { return EBTNodeResult::Failed; }

        bAttackLaunched = true;
        SubscribeAttackFinished(OwnerComp, Boss);
    }

    return EBTNodeResult::InProgress;
}

void UBTTask_SevarogRush::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    // OnAttackFinished delegate handles completion once the attack is launched.
    if (bAttackLaunched) { return; }

    ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
    if (!IsValid(Controller)) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

    ARVSevarogCharacter* Boss   = Controller->GetBossCharacter();
    APawn*               Player = Controller->GetPlayerPawn();
    if (!IsValid(Boss) || !IsValid(Player)) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

    const float Dist = FVector::Dist(Boss->GetActorLocation(), Player->GetActorLocation());

    if (Dist <= Boss->GetSevarogData()->ArrivalRange)
    {
        Controller->StopMovement();
        Boss->EndRush();

        if (!LaunchAttack(Boss))
        {
            FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
            return;
        }

        bAttackLaunched = true;
        SubscribeAttackFinished(OwnerComp, Boss);
    }
}

EBTNodeResult::Type UBTTask_SevarogRush::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::AbortTask(OwnerComp, NodeMemory);

    ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
    if (IsValid(Controller)) { Controller->StopMovement(); }

    return EBTNodeResult::Aborted;
}

void UBTTask_SevarogRush::OnTaskFinished(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    // Base cleans up the OnAttackFinished delegate and calls ForceEndCurrentAction if still attacking.
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

    ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
    if (!IsValid(Controller)) { return; }

    ARVSevarogCharacter* Boss = Controller->GetBossCharacter();
    if (!IsValid(Boss)) { return; }

    if (Boss->IsRushing()) { Boss->EndRush(); }
}