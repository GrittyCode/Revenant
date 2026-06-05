#include "AI/BTTask_SevarogPhaseAttack.h"
#include "Character/Enemy/RVSevarogCharacter.h"

UBTTask_SevarogPhaseAttack::UBTTask_SevarogPhaseAttack()
{
	NodeName = TEXT("Sevarog Phase Attack");
}

bool UBTTask_SevarogPhaseAttack::LaunchAttack(ARVSevarogCharacter* InBoss)
{
	return InBoss->ExecutePhaseAttack();
}