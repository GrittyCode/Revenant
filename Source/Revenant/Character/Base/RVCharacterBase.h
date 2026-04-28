// Source/Revenant/Character/Base/RVCharacterBase.h
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
};