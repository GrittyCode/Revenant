#include "AI/RVBTService_SevarogState.h"
#include "AI/RVAIController.h"
#include "AI/RVSevarogBlackBoardKeys.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "Data/RVSevarogDataAsset.h"

UBTService_RVSevarogState::UBTService_RVSevarogState()
{
	NodeName        = TEXT("Sevarog State");
	Interval        = 0.1f;
	RandomDeviation = 0.f;
}

void UBTService_RVSevarogState::OnSearchStart(FBehaviorTreeSearchData& SearchData)
{
	UBehaviorTreeComponent& OwnerComp = SearchData.OwnerComp;

	ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(Controller)) { return; }

	ARVSevarogCharacter* Boss = Controller->GetBossCharacter();
	if (!IsValid(Boss)) { return; }

	const URVSevarogDataAsset* Data = Boss->GetSevarogData();
	if (!IsValid(Data)) { return; }

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!IsValid(BB)) { return; }

	
	BB->SetValueAsFloat(RVSevarogBlackboardKeys::ArrivalRange, Data->ArrivalRange);
	BB->SetValueAsFloat(RVSevarogBlackboardKeys::RushCooldownDuration,        Data->RushCooldown);
	BB->SetValueAsFloat(RVSevarogBlackboardKeys::SoulSiphonCooldownDuration,  Data->SoulSiphonCooldown);
	BB->SetValueAsFloat(RVSevarogBlackboardKeys::SubjugationCooldownDuration, Data->SubjugationCooldown);
}

void UBTService_RVSevarogState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(Controller)) { return; }

	ARVSevarogCharacter* Boss = Controller->GetBossCharacter();
	if (!IsValid(Boss)) { return; }

	APawn* Player = Controller->GetPlayerPawn();

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!IsValid(BB)) { return; }

	BB->SetValueAsObject(RVSevarogBlackboardKeys::PlayerPawn, Player);

	const float Dist = IsValid(Player)
		? FVector::Dist(Boss->GetActorLocation(), Player->GetActorLocation())
		: BIG_NUMBER;

	const URVSevarogDataAsset* Data = Boss->GetSevarogData();
	
	BB->SetValueAsBool(RVSevarogBlackboardKeys::bIsRushRadius,   Dist >= Data->RushTriggerRadius);
	BB->SetValueAsBool(RVSevarogBlackboardKeys::bIsAttackRadius, Dist <= Data->MeleeEngagementRange);
	BB->SetValueAsBool(RVSevarogBlackboardKeys::bIsGroggy,    Boss->IsGroggy());
	
	BB->SetValueAsEnum(RVSevarogBlackboardKeys::CurrentPhase, static_cast<uint8>(Boss->GetCurrentPhase()));
}