// Source/Revenant/Component/RVAttributeComponent.cpp
#include "Component/RVAttributeComponent.h"
#include "Data/RVCharacterDataAsset.h"

URVAttributeComponent::URVAttributeComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URVAttributeComponent::BeginPlay()
{
    Super::BeginPlay();

    // Fallback init — overwritten by InitFromDataAsset if CharacterData is set
    CurrentHealth  = MaxHealth;
    CurrentStamina = MaxStamina;

    // Start regen from the beginning of play
    ResumeStaminaRegen();
}

void URVAttributeComponent::InitFromDataAsset(URVCharacterDataAsset* InData)
{
    if (!IsValid(InData)) { return; }

    MaxHealth        = InData->MaxHealth;
    MaxStamina       = InData->MaxStamina;
    StaminaRegenRate = InData->StaminaRegenRate;
    StaminaRegenDelay = InData->StaminaRegenDelay;

    CurrentHealth  = MaxHealth;
    CurrentStamina = MaxStamina;
}

// ─── HP ──────────────────────────────────────────────────────────────────────

bool URVAttributeComponent::ApplyDamage(AActor* InInstigator, float InDamageAmount)
{
    if (!IsAlive()) { return false; }

    const float Clamped = FMath::Max(0.f, InDamageAmount);
    CurrentHealth = FMath::Max(0.f, CurrentHealth - Clamped);
    OnHealthChanged.Broadcast(CurrentHealth, -Clamped);

    if (CurrentHealth <= 0.f)
    {
        OnDeath.Broadcast();
        return false;
    }
    return true;
}

bool URVAttributeComponent::ApplyHealing(float InHealAmount)
{
    if (!IsAlive()) { return false; }

    const float Clamped = FMath::Max(0.f, InHealAmount);
    const float Delta   = FMath::Min(Clamped, MaxHealth - CurrentHealth);
    CurrentHealth += Delta;
    OnHealthChanged.Broadcast(CurrentHealth, Delta);
    return true;
}

float URVAttributeComponent::GetHealthPercent() const
{
    return MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f;
}

bool URVAttributeComponent::IsAlive() const
{
    return CurrentHealth > 0.f;
}

// ─── Stamina ─────────────────────────────────────────────────────────────────

bool URVAttributeComponent::ConsumeStamina(float InAmount)
{
    if (CurrentStamina < InAmount) { return false; }

    CurrentStamina = FMath::Max(0.f, CurrentStamina - InAmount);
    OnStaminaChanged.Broadcast(CurrentStamina, -InAmount);
    return true;
}

bool URVAttributeComponent::ApplyStaminaDamage(float InAmount)
{
    const float Clamped = FMath::Max(0.f, InAmount);
    CurrentStamina = FMath::Max(0.f, CurrentStamina - Clamped);
    OnStaminaChanged.Broadcast(CurrentStamina, -Clamped);

    if (CurrentStamina <= 0.f)
    {
        // Publisher perspective: "stamina hit zero" — subscriber decides what this means.
        OnStaminaDepleted.Broadcast();
        return false; // guard broke (caller interprets context)
    }
    return true; // stamina still remains
}

void URVAttributeComponent::PauseStaminaRegen()
{
    UWorld* World = GetWorld();
    if (!IsValid(World)) { return; }

    World->GetTimerManager().ClearTimer(StaminaRegenDelayHandle);
    World->GetTimerManager().ClearTimer(StaminaRegenHandle);
}

void URVAttributeComponent::ResumeStaminaRegen()
{
    UWorld* World = GetWorld();
    if (!IsValid(World)) { return; }

    // Restart the delay before regen begins
    World->GetTimerManager().SetTimer(
        StaminaRegenDelayHandle,
        this,
        &URVAttributeComponent::StartStaminaRegenTick,
        StaminaRegenDelay,
        false
    );
}

void URVAttributeComponent::StartStaminaRegenTick()
{
    UWorld* World = GetWorld();
    if (!IsValid(World)) { return; }

    World->GetTimerManager().SetTimer(
        StaminaRegenHandle,
        this,
        &URVAttributeComponent::TickStaminaRegen,
        StaminaRegenInterval,
        true
    );
}

void URVAttributeComponent::TickStaminaRegen()
{
    if (CurrentStamina >= MaxStamina)
    {
        PauseStaminaRegen();
        return;
    }

    const float Delta = FMath::Min(StaminaRegenRate, MaxStamina - CurrentStamina);
    CurrentStamina += Delta;
    OnStaminaChanged.Broadcast(CurrentStamina, Delta);
}

float URVAttributeComponent::GetStaminaPercent() const
{
    return MaxStamina > 0.f ? CurrentStamina / MaxStamina : 0.f;
}