#include "Component/RVAttributeComponent.h"
#include "Data/RVCharacterStatRow.h"

URVAttributeComponent::URVAttributeComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URVAttributeComponent::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth  = MaxHealth;
    CurrentStamina = MaxStamina;
    CurrentPoise   = MaxPoise;

    ResumeStaminaRegen();
}

void URVAttributeComponent::InitFromStatRow(const FRVCharacterStatRow& InRow)
{
    MaxHealth         = InRow.MaxHealth;
    MaxStamina        = InRow.MaxStamina;
    StaminaRegenRate  = InRow.StaminaRegenRate;
    StaminaRegenDelay = InRow.StaminaRegenDelay;
    MaxPoise          = InRow.MaxPoise;
    PoiseRegenDelay   = InRow.PoiseRegenDelay;

    CurrentHealth  = MaxHealth;
    CurrentStamina = MaxStamina;
    CurrentPoise   = MaxPoise;
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
    if (CurrentStamina <= 0.f) { return false; }

    CurrentStamina = FMath::Max(0.f, CurrentStamina - InAmount);
    OnStaminaChanged.Broadcast(CurrentStamina, -InAmount);

    ResetStaminaRegenDelay();
    return true;
}

bool URVAttributeComponent::ApplyStaminaDamage(float InAmount)
{
    const float Clamped = FMath::Max(0.f, InAmount);
    CurrentStamina = FMath::Max(0.f, CurrentStamina - Clamped);
    OnStaminaChanged.Broadcast(CurrentStamina, -Clamped);

    ResetStaminaRegenDelay();

    if (CurrentStamina <= 0.f)
    {
        OnStaminaDepleted.Broadcast();
        return false;
    }
    return true;
}

void URVAttributeComponent::ResetStaminaRegenDelay()
{
    PauseStaminaRegen();
    ResumeStaminaRegen();
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

    World->GetTimerManager().SetTimer(
        StaminaRegenDelayHandle,
        this,
        &URVAttributeComponent::StartStaminaRegenTick,
        StaminaRegenDelay,
        false);
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
        true);
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

// ─── Poise ───────────────────────────────────────────────────────────────────

bool URVAttributeComponent::ApplyPoiseDamage(float InPoiseDamage)
{
    const float Clamped = FMath::Max(0.f, InPoiseDamage);
    CurrentPoise = FMath::Max(0.f, CurrentPoise - Clamped);

    OnPoiseChanged.Broadcast(GetPoiseRatio());

    // Reset regen delay on every hit.
    UWorld* World = GetWorld();
    if (IsValid(World) && PoiseRegenDelay > 0.f)
    {
        World->GetTimerManager().ClearTimer(PoiseRegenDelayHandle);
        World->GetTimerManager().SetTimer(
            PoiseRegenDelayHandle,
            this,
            &URVAttributeComponent::StartPoiseRegen,
            PoiseRegenDelay,
            false);
    }

    if (CurrentPoise <= 0.f)
    {
        OnPoiseDepleted.Broadcast();
        return true;
    }
    return false;
}

void URVAttributeComponent::ResetPoise()
{
    UWorld* World = GetWorld();
    if (IsValid(World))
    {
        World->GetTimerManager().ClearTimer(PoiseRegenDelayHandle);
    }

    CurrentPoise = MaxPoise;
    OnPoiseChanged.Broadcast(1.f);
}

void URVAttributeComponent::StartPoiseRegen()
{
    CurrentPoise = MaxPoise;
    OnPoiseChanged.Broadcast(1.f);
}