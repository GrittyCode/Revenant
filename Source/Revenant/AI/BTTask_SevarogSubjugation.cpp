#include "AI/BTTask_SevarogSubjugation.h"
#include "Character/Enemy/RVSevarogCharacter.h"

UBTTask_SevarogSubjugation::UBTTask_SevarogSubjugation()
{
	NodeName = TEXT("Sevarog Subjugation");
}

bool UBTTask_SevarogSubjugation::LaunchAttack(ARVSevarogCharacter* InBoss)
{
	return InBoss->ExecuteSubjugation();
}