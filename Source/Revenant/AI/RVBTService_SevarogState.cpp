// Source/Revenant/AI/RVBTService_SevarogState.cpp
#include "AI/RVBTService_SevarogState.h"
#include "AI/RVAIController.h"
#include "AI/RVSevarogBlackBoardKeys.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "Data/Asset/RVSevarogDataAsset.h"

UBTService_RVSevarogState::UBTService_RVSevarogState()
{
    NodeName        = TEXT("Sevarog State");
    Interval        = 0.1f;
    RandomDeviation = 0.f;
}

void UBTService_RVSevarogState::OnSearchStart(FBehaviorTreeSearchData& SearchData)
{
    UBehaviorTreeComponent& OwnerComp = SearchData.OwnerComp;

    const ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
    if (!IsValid(Controller)) { return; }

    const ARVSevarogCharacter* Boss = Controller->GetBossCharacter();
    if (!IsValid(Boss)) { return; }

    const URVSevarogDataAsset* Data = Boss->GetSevarogData();
    if (!IsValid(Data)) { return; }

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!IsValid(BB)) { return; }

    // Static values written once — these do not change at runtime.
    BB->SetValueAsFloat(RVSevarogBlackboardKeys::ArrivalRange,                Data->ArrivalRange);
    BB->SetValueAsFloat(RVSevarogBlackboardKeys::RushCooldownDuration,        Data->RushCooldown);
    BB->SetValueAsFloat(RVSevarogBlackboardKeys::SoulSiphonCooldownDuration,  Data->SoulSiphon.Cooldown);
    BB->SetValueAsFloat(RVSevarogBlackboardKeys::SubjugationCooldownDuration, Data->Subjugation.Cooldown);
}

void UBTService_RVSevarogState::TickNode(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    const ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
    if (!IsValid(Controller)) { return; }

    const ARVSevarogCharacter* Boss = Controller->GetBossCharacter();
    if (!IsValid(Boss)) { return; }

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!IsValid(BB)) { return; }

    APawn* Player = Controller->GetPlayerPawn();
    BB->SetValueAsObject(RVSevarogBlackboardKeys::PlayerPawn, Player);

    const float Dist = IsValid(Player)
        ? FVector::Dist(Boss->GetActorLocation(), Player->GetActorLocation())
        : BIG_NUMBER;

    const URVSevarogDataAsset* Data = Boss->GetSevarogData();

    BB->SetValueAsBool(RVSevarogBlackboardKeys::bIsGroggy,            Boss->IsGroggy());
    BB->SetValueAsBool(RVSevarogBlackboardKeys::bIsAttacking,         Boss->IsAttacking());
    BB->SetValueAsBool(RVSevarogBlackboardKeys::bIsRushRadius,        Dist >= Data->RushTriggerRadius);
    BB->SetValueAsBool(RVSevarogBlackboardKeys::bIsAttackRadius,      Dist <= Data->MeleeEngagementRange);
    BB->SetValueAsBool(RVSevarogBlackboardKeys::bIsSoulSiphonRadius,  Dist <= Data->SoulSiphon.EngagementRange);
    BB->SetValueAsBool(RVSevarogBlackboardKeys::bIsSubjugationRadius, Dist <= Data->Subjugation.EngagementRange);
    BB->SetValueAsEnum(RVSevarogBlackboardKeys::CurrentPhase, static_cast<uint8>(Boss->GetCurrentPhase()));
}