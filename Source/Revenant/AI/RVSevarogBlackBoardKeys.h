#pragma once

#include "CoreMinimal.h"

// Blackboard key names for BB_SevarogAI.
namespace RVSevarogBlackboardKeys
{
	static const FName PlayerPawn                  = TEXT("PlayerPawn");
	static const FName CurrentPhase                = TEXT("CurrentPhase");
	static const FName bIsGroggy                   = TEXT("bIsGroggy");
	static const FName bIsRushRadius               = TEXT("bIsRushRadius");
	static const FName bIsAttackRadius             = TEXT("bIsAttackRadius");
	static const FName bIsSoulSiphonRadius         = TEXT("bIsSoulSiphonRadius");
	static const FName bIsSubjugationRadius        = TEXT("bIsSubjugationRadius");
	static const FName RushCooldownDuration        = TEXT("RushCooldownDuration");
	static const FName SoulSiphonCooldownDuration  = TEXT("SoulSiphonCooldownDuration");
	static const FName SubjugationCooldownDuration = TEXT("SubjugationCooldownDuration");
	static const FName ArrivalRange                = TEXT("ArrivalRange");
}