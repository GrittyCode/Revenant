#include "AI/BTTask_SevarogSoulsiphon.h"
#include "Character/Enemy/RVSevarogCharacter.h"

UBTTask_SevarogSoulsiphon::UBTTask_SevarogSoulsiphon()
{
	NodeName = TEXT("Sevarog Soul Siphon");
}

bool UBTTask_SevarogSoulsiphon::LaunchAttack(ARVSevarogCharacter* InBoss)
{
	return InBoss->ExecuteSoulSiphon();
}