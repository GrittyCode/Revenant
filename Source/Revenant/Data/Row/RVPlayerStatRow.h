#pragma once

#include "CoreMinimal.h"
#include "Data/Row/RVCharacterStatRow.h"
#include "RVPlayerStatRow.generated.h"

// Player-only stats stored in DT_PlayerStats.
// Extends FRVCharacterStatRow so InitFromStatRow(const FRVCharacterStatRow&) accepts this by base ref.
USTRUCT(BlueprintType)
struct REVENANT_API FRVPlayerStatRow : public FRVCharacterStatRow
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float MaxStamina = 100.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float StaminaRegenRate = 20.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float StaminaRegenDelay = 1.5f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float DodgeStaminaCost = 30.f;
};
