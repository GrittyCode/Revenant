#include "AI/BTTask_SevarogAttackBase.h"
#include "AI/RVAIController.h"
#include "Character/Enemy/RVSevarogCharacter.h"

UBTTask_SevarogAttackBase::UBTTask_SevarogAttackBase()
{
    bCreateNodeInstance = true;
    bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_SevarogAttackBase::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
    if (!IsValid(Controller)) { return EBTNodeResult::Failed; }

    ARVSevarogCharacter* Boss = Controller->GetBossCharacter();
    if (!IsValid(Boss)) { return EBTNodeResult::Failed; }

    // Bind before launch — see SubscribeAttackFinished.
    SubscribeAttackFinished(OwnerComp, Boss);

    if (!LaunchAttack(Boss))
    {
        Boss->OnAttackFinished.RemoveAll(this);
        return EBTNodeResult::Failed;
    }

    return EBTNodeResult::InProgress;
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