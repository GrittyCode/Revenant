#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/RVCombatInterface.h"
#include "Interface/RVDamageable.h"
#include "RVCharacterBase.generated.h"

class URVAttributeComponent;
class URVEquipmentComponent;
class URVCombatStateComponent;
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
     * Routes incoming damage:
     *   Invincible (i-frame) → blocked entirely
     *   Otherwise           → HP damage via AttributeComponent,
     *                         then hit reaction via HitReactionComponent
     *
     * Guard routing is handled by ARVCharacterPlayer::ApplyDamage override.
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
    TObjectPtr<URVEquipmentComponent> EquipmentComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVCombatStateComponent> CombatStateComponent;

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