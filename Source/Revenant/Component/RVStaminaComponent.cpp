#include "Component/RVStaminaComponent.h"
#include "Data/RVPlayerStatRow.h"

URVStaminaComponent::URVStaminaComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URVStaminaComponent::InitFromStatRow(const FRVPlayerStatRow& InRow)
{
    MaxStamina        = InRow.MaxStamina;
    StaminaRegenRate  = InRow.StaminaRegenRate;
    StaminaRegenDelay = InRow.StaminaRegenDelay;
    CurrentStamina    = MaxStamina;

    PauseStaminaRegen();
    ResumeStaminaRegen();
}

bool URVStaminaComponent::ConsumeStamina(float InAmount)
{
    if (CurrentStamina < InAmount) { return false; }
    const float Prev   = CurrentStamina;
    CurrentStamina     = FMath::Max(0.f, CurrentStamina - InAmount);
    OnStaminaChanged.Broadcast(CurrentStamina, CurrentStamina - Prev);
    ResetStaminaRegenDelay();
    return true;
}

bool URVStaminaComponent::ApplyStaminaDamage(float InAmount)
{
    const float Prev = CurrentStamina;
    CurrentStamina   = FMath::Max(0.f, CurrentStamina - InAmount);
    OnStaminaChanged.Broadcast(CurrentStamina, CurrentStamina - Prev);
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
    if (!IsValid(World)) { return; }
    World->GetTimerManager().ClearTimer(StaminaRegenDelayHandle);
    World->GetTimerManager().ClearTimer(StaminaRegenHandle);
}

void URVStaminaComponent::ResumeStaminaRegen()
{
    UWorld* World = GetWorld();
    if (!IsValid(World)) { return; }
    World->GetTimerManager().SetTimer(
        StaminaRegenDelayHandle, this,
        &URVStaminaComponent::StartStaminaRegenTick,
        StaminaRegenDelay, false);
}

void URVStaminaComponent::StartStaminaRegenTick()
{
    UWorld* World = GetWorld();
    if (!IsValid(World)) { return; }
    World->GetTimerManager().SetTimer(
        StaminaRegenHandle, this,
        &URVStaminaComponent::TickStaminaRegen,
        StaminaRegenInterval, true);
}

void URVStaminaComponent::TickStaminaRegen()
{
    if (CurrentStamina >= MaxStamina) { PauseStaminaRegen(); return; }
    const float Prev   = CurrentStamina;
    const float Delta  = FMath::Min(StaminaRegenRate * StaminaRegenInterval, MaxStamina - CurrentStamina);
    CurrentStamina    += Delta;
    OnStaminaChanged.Broadcast(CurrentStamina, CurrentStamina - Prev);
}
