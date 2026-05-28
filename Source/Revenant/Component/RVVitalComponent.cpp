#include "Component/RVVitalComponent.h"
#include "Data/RVCharacterStatRow.h"

URVVitalComponent::URVVitalComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URVVitalComponent::InitFromStatRow(const FRVCharacterStatRow& InRow)
{
    MaxHealth       = InRow.MaxHealth;
    MaxPoise        = InRow.MaxPoise;
    PoiseRegenDelay = InRow.PoiseRegenDelay;
    CurrentHealth   = MaxHealth;
    CurrentPoise    = MaxPoise;
}

//--- Health ------------------------------------------------------------------

bool URVVitalComponent::ApplyDamage(AActor* InInstigator, float InDamageAmount)
{
    if (!IsAlive()) { return false; }

    CurrentHealth = FMath::Max(0.f, CurrentHealth - FMath::Max(0.f, InDamageAmount));
    OnHealthChanged.Broadcast(GetHealthPercent());

    if (CurrentHealth <= 0.f) { OnDeath.Broadcast(); return false; }
    return true;
}

bool URVVitalComponent::ApplyHealing(float InHealAmount)
{
    if (!IsAlive()) { return false; }

    CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + FMath::Max(0.f, InHealAmount));
    OnHealthChanged.Broadcast(GetHealthPercent());
    return true;
}

float URVVitalComponent::GetHealthPercent() const
{
    return MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f;
}

//--- Poise -------------------------------------------------------------------

void URVVitalComponent::ApplyPoiseDamage(float InPoiseDamage)
{
    CurrentPoise = FMath::Max(0.f, CurrentPoise - FMath::Max(0.f, InPoiseDamage));
    OnPoiseChanged.Broadcast(GetPoiseRatio());

    UWorld* World = GetWorld();
    if (IsValid(World) && PoiseRegenDelay > 0.f)
    {
        World->GetTimerManager().ClearTimer(PoiseRegenDelayHandle);
        World->GetTimerManager().SetTimer(
            PoiseRegenDelayHandle, this,
            &URVVitalComponent::StartPoiseRegen,
            PoiseRegenDelay, false);
    }

    if (CurrentPoise <= 0.f) { OnPoiseDepleted.Broadcast(); }
}

void URVVitalComponent::ResetPoise()
{
    UWorld* World = GetWorld();
    if (IsValid(World)) { World->GetTimerManager().ClearTimer(PoiseRegenDelayHandle); }

    CurrentPoise = MaxPoise;
    OnPoiseChanged.Broadcast(1.f);
}

void URVVitalComponent::StartPoiseRegen()
{
    CurrentPoise = MaxPoise;
    OnPoiseChanged.Broadcast(1.f);
}