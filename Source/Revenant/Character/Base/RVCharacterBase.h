#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/RVCombatInterface.h"
#include "Interface/RVDamageable.h"
#include "RVCharacterBase.generated.h"

class URVAttributeComponent;
class URVComboComponent;
class URVEquipmentComponent;
class URVCombatStateComponent;
class URVHeavyAttackComponent;
class URVDodgeComponent;
class URVGuardComponent;
class URVSprintComponent;
class URVHitReactionComponent;
class URVCharacterDataAsset;

UCLASS()
class REVENANT_API ARVCharacterBase : public ACharacter, public IRVCombatInterface, public IRVDamageable
{
    GENERATED_BODY()

public:
    ARVCharacterBase();

    // --- IRVCombatInterface --------------------------------------------------

    /** Delegates to URVCombatStateComponent::PerformAttackTrace(). */
    virtual void ActivateHitCheck() override;

    // --- IRVDamageable -------------------------------------------------------

    /**
     * Routes incoming damage based on current combat state:
     *   Invincible (i-frame)  → blocked entirely
     *   Guarding              → stamina damage via GuardComponent (may trigger guard break)
     *   Normal                → HP damage via AttributeComponent,
     *                           then hit reaction via HitReactionComponent
     */
    virtual bool ApplyDamage(const FRVHitInfo& InHitInfo) override;

protected:
    virtual void BeginPlay() override;

    virtual void Falling() override;
    virtual void Landed(const FHitResult& Hit) override;

    // --- Components ----------------------------------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVAttributeComponent> AttributeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVComboComponent> ComboComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVEquipmentComponent> EquipmentComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVCombatStateComponent> CombatStateComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVHeavyAttackComponent> HeavyAttackComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVDodgeComponent> DodgeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVGuardComponent> GuardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVSprintComponent> SprintComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVHitReactionComponent> HitReactionComponent;

    // --- Data ----------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Data")
    TObjectPtr<URVCharacterDataAsset> CharacterData;

    // --- Movement ------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "RV|Movement")
    FRotator AirRotationRate = FRotator(0.f, 0.f, 0.f);

private:
    FRotator OriginalRotationRate;
};