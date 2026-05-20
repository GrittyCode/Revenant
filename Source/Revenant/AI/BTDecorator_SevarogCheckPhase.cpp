#include "AI/BTDecorator_SevarogCheckPhase.h"
#include "AI/RVAIController.h"
#include "Character/Enemy/RVSevarogCharacter.h"

UBTDecorator_SevarogCheckPhase::UBTDecorator_SevarogCheckPhase()
{
	NodeName = TEXT("Sevarog Check Phase");
}

bool UBTDecorator_SevarogCheckPhase::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	Super::CalculateRawConditionValue(OwnerComp, NodeMemory);
	
	const ARVAIController* Controller = Cast<ARVAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(Controller)) { return false; }

	const ARVSevarogCharacter* Boss = Controller->GetBossCharacter();
	if (!IsValid(Boss)) { return false; }

	return Boss->GetCurrentPhase() >= RequiredPhase;
}

FString UBTDecorator_SevarogCheckPhase::GetStaticDescription() const
{
	Super::GetStaticDescription();
	
	const UEnum* PhaseEnum = StaticEnum<ERVBossPhase>();
	const FString PhaseName = PhaseEnum
		? PhaseEnum->GetNameStringByValue(static_cast<int64>(RequiredPhase))
		: TEXT("Unknown");
	return FString::Printf(TEXT("Phase >= %s"), *PhaseName);
}