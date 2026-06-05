#include "AI/BTTask_SevarogWaitGroggyEnd.h"
#include "AI/RVAIController.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "Component/Combat/RVHitReactionComponent.h"

UBTTask_SevarogWaitGroggyEnd::UBTTask_SevarogWaitGroggyEnd()
{
    NodeName            = TEXT("Sevarog Wait Groggy End");
    bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_SevarogWaitGroggyEnd::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
    if (!IsValid(Controller)) { return EBTNodeResult::Failed; }

    ARVSevarogCharacter* Boss = Controller->GetBossCharacter();
    if (!IsValid(Boss)) { return EBTNodeResult::Failed; }

    // Already recovered before this task ran — succeed immediately.
    if (!Boss->IsGroggy()) { return EBTNodeResult::Succeeded; }

    URVHitReactionComponent* HitReaction = Boss->GetHitReactionComponent();
    if (!IsValid(HitReaction)) { return EBTNodeResult::Failed; }

    TWeakObjectPtr<UBehaviorTreeComponent>  BTWeak(&OwnerComp);
    TWeakObjectPtr<URVHitReactionComponent> HitReactionWeak(HitReaction);

    // ARVSevarogCharacter::OnGroggySequenceCompleted is subscribed first (BeginPlay),
    // so state cleanup always runs before this lambda fires.
    HitReaction->OnGroggySequenceCompleted.AddWeakLambda(this, [BTWeak, HitReactionWeak, this]()
    {
        if (HitReactionWeak.IsValid()) { HitReactionWeak->OnGroggySequenceCompleted.RemoveAll(this); }
        if (BTWeak.IsValid())          { FinishLatentTask(*BTWeak.Get(), EBTNodeResult::Succeeded); }
    });

    return EBTNodeResult::InProgress;
}

void UBTTask_SevarogWaitGroggyEnd::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

    ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
    if (!IsValid(Controller)) { return; }

    ARVSevarogCharacter* Boss = Controller->GetBossCharacter();
    if (!IsValid(Boss)) { return; }

    URVHitReactionComponent* HitReaction = Boss->GetHitReactionComponent();
    if (!IsValid(HitReaction)) { return; }

    // Guard against double-remove (lambda may have already cleaned up on natural completion).
    HitReaction->OnGroggySequenceCompleted.RemoveAll(this);
}
