#include "Component/Attribute/RVStaminaComponent.h"
#include "Data/Row/RVPlayerStatRow.h"

URVStaminaComponent::URVStaminaComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URVStaminaComponent::InitFromStatRow(const FRVPlayerStatRow& InRow)
{
    MaxStamina           = InRow.MaxStamina;
    StaminaRegenRate     = InRow.StaminaRegenRate;
    StaminaRegenDelay    = InRow.StaminaRegenDelay;
    StaminaRegenInterval = InRow.StaminaRegenInterval;
    CurrentStamina       = MaxStamina;

    PauseStaminaRegen();
    ResumeStaminaRegen();
}

bool URVStaminaComponent::ConsumeStamina(float InAmount)
{
    if (CurrentStamina < InAmount) { return false; }
    CurrentStamina = FMath::Max(0.f, CurrentStamina - InAmount);
    OnStaminaChanged.Broadcast(GetStaminaPercent());
    ResetStaminaRegenDelay();
    return true;
}

bool URVStaminaComponent::ApplyStaminaDamage(float InAmount)
{
    CurrentStamina = FMath::Max(0.f, CurrentStamina - InAmount);
    OnStaminaChanged.Broadcast(GetStaminaPercent());
    ResetStaminaRegenDelay();

    if (CurrentStamina <= 0.f) { OnStaminaDepleted.Broadcast(); return false; }
    return true;
}

float URVStaminaComponent::GetStaminaPercent() const
{
    return MaxStamina > 0.f ? CurrentStamina / MaxStamina : 0.f;
}

void URVStaminaComponent::ResetStaminaRegenDelay()
{
    PauseStaminaRegen();
    ResumeStaminaRegen();
}

void URVStaminaComponent::PauseStaminaRegen()
{
    UWorld* World = GetWorld();
    World->GetTimerManager().ClearTimer(StaminaRegenDelayHandle);
    World->GetTimerManager().ClearTimer(StaminaRegenHandle);
}

void URVStaminaComponent::ResumeStaminaRegen()
{
    UWorld* World = GetWorld();
    World->GetTimerManager().SetTimer(
        StaminaRegenDelayHandle, this,
        &URVStaminaComponent::StartStaminaRegenTick,
        StaminaRegenDelay, false);
}

void URVStaminaComponent::StartStaminaRegenTick()
{
    UWorld* World = GetWorld();
    World->GetTimerManager().SetTimer(
        StaminaRegenHandle, this,
        &URVStaminaComponent::TickStaminaRegen,
        StaminaRegenInterval, true);
}

void URVStaminaComponent::TickStaminaRegen()
{
    if (CurrentStamina >= MaxStamina) { PauseStaminaRegen(); return; }
    const float Delta  = FMath::Min(StaminaRegenRate * StaminaRegenInterval, MaxStamina - CurrentStamina);
    CurrentStamina    += Delta;
    OnStaminaChanged.Broadcast(GetStaminaPercent());
}
