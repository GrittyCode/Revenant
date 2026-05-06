#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/RVCombatInterface.h"
#include "Interface/RVDamageable.h"
#include "RVCharacterBase.generated.h"

class URVAttributeComponent;
class URVComboComponent;
class URVEquipmentComponent;
class URVCombatComponent;
class URVCharacterDataAsset;

UCLASS()
class REVENANT_API ARVCharacterBase : public ACharacter, public IRVCombatInterface, public IRVDamageable
{
    GENERATED_BODY()

public:
    ARVCharacterBase();

    // --- IRVCombatInterface ---------------------------------------------------------

    /** Delegates to URVCombatComponent::PerformAttackTrace(). */
    virtual void ActivateHitCheck() override;

    // --- IRVDamageable ---------------------------------------------------------

    /**
     * Routes incoming damage based on current combat state:
     *   Invincible (i-frame) → blocked
     *   Guarding             → stamina damage (may trigger guard break)
     *   Default              → HP damage
     */
    virtual bool ApplyDamage(float InDamageAmount, AActor* InInstigator) override;

    virtual void OnHitReaction(FVector InHitDirection) override;

protected:
    virtual void BeginPlay() override;

    /**
     * Gate checks CombatComponent, then delegates to ComboComponent::HandleComboInput().
     * Centralizes the action gate so ComboComponent has no dependency on CombatComponent.
     */
    void TryStartCombo();

    // Reduces RotationRate.Yaw on airborne entry to prevent free mid-air steering.
    // OriginalRotationRate is cached here and restored exactly on Landed().
    virtual void Falling() override;
    virtual void Landed(const FHitResult& Hit) override;

    // --- Components ---------------------------------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVAttributeComponent> AttributeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVComboComponent> ComboComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVEquipmentComponent> EquipmentComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVCombatComponent> CombatComponent;

    // --- Data ---------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Data")
    TObjectPtr<URVCharacterDataAsset> CharacterData;

    // --- Movement ---------------------------------------------------------

    /**
     * RotationRate applied while airborne.
     * Low Yaw gives the character a heavy, committed feel during jumps.
     * Tunable per BP subclass in the Details panel.
     */
    UPROPERTY(EditDefaultsOnly, Category = "RV|Movement")
    FRotator AirRotationRate = FRotator(0.f, 0.0f, 0.f);

private:
    // Cached at Falling() entry — restored exactly on Landed() regardless of
    // what RotationRate was set to (sprint, guard, etc.)
    FRotator OriginalRotationRate;
};