#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/RVHitCheckTarget.h"
#include "Interface/RVDamageable.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "RVCharacterBase.generated.h"

class URVHitReactionComponent;
class URVAttackTraceComponent;
class URVCharacterDataAsset;
class URVHitReactionAnimDataAsset;
class UMeshComponent;
class UNiagaraSystem;
class UParticleSystem;
class USoundBase;

UCLASS()
class REVENANT_API ARVCharacterBase : public ACharacter, public IRVHitCheckTarget, public IRVDamageable
{
    GENERATED_BODY()

public:
    ARVCharacterBase();

    virtual void ActivateHitCheck() override;
    virtual bool ApplyDamage(const FRVHitInfo& InHitInfo) override;

    //--- Attribute queries (external: GameMode, Widget) ----------------------

    UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
    float GetHealthRatio() const;

    UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
    float GetStaminaRatio() const;

    //--- Attribute event facades (external: GameMode, Widget) ----------------

    FRVOnHealthChanged&  GetOnHealthChanged();
    FRVOnStaminaChanged& GetOnStaminaChanged();
    FRVOnDeath&          GetOnDeath();
    FRVOnPoiseDepleted&  GetOnPoiseDepleted();
    FRVOnPoiseChanged&   GetOnPoiseChanged();

    //--- AnimNotify entry points (CharacterBase interface for polymorphic notify dispatch) ---

    void OpenAttackHitWindow();
    void CloseAttackHitWindow();

    virtual void ActivateWeaponTrail()   {}
    virtual void DeactivateWeaponTrail() {}

    //--- AnimInstance state queries ------------------------------------------

    bool  IsInCombatState(ERVCombatState InState) const;
    float GetStaggerDirection() const;

    //--- Combat state operations (components and subclasses call through here) ---

    void AddCombatState(ERVCombatState InState);
    void RemoveCombatState(ERVCombatState InState);
    bool HasCombatState(ERVCombatState InState) const;
    bool CanAct(ERVCombatState InCoexistableStates = ERVCombatState::None) const;
    bool IsGrounded() const;
    void SetInvincible(bool bInvincible);
    bool IsInvincible() const;
    void ForceEndAllActions();

    //--- Stamina operations --------------------------------------------------

    bool  TryConsumeStamina(float InAmount);
    float GetCurrentStamina() const;
    void  PauseStaminaRegen();
    void  ResumeStaminaRegen();
    void  ResetStaminaRegenDelay();
    bool  ApplyStaminaDamage(float InAmount);

    //--- Poise operations ----------------------------------------------------

    float GetMaxPoise() const;
    float GetPoiseRatio() const;
    bool  ApplyPoiseDamage(float InAmount);
    void  ResetPoise();

    //--- Attack trace operations (routes to URVAttackTraceComponent) ---------

    void SetCombatStat(float InBaseDamage, float InBasePoiseDamage, float InAttackRadius);
    void SetHitFX(UNiagaraSystem* InNiagara, UParticleSystem* InCascade, USoundBase* InSFX);

    //--- Hit reaction operations (routes to URVHitReactionComponent) ---------

    void TriggerStaggerWithMontage(UAnimMontage* InMontage);

protected:
    virtual void BeginPlay() override;
    virtual void Falling() override;
    virtual void Landed(const FHitResult& Hit) override;

    virtual URVHitReactionAnimDataAsset* GetHitReactionAnimData() const { return nullptr; }
    virtual void InitStats() {}

    UFUNCTION()
    virtual void OnDeath();

    // Override to supply a different trace mesh (e.g. weapon mesh for Player).
    // Base implementation returns the skeletal mesh.
    virtual UMeshComponent* GetWeaponTraceMesh() const { return GetMesh(); }

    FVector GetForwardLocation(float InOffset = 1.f) const;
    FVector GetGroundOrigin() const;

    //--- Components ----------------------------------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVAttributeComponent> AttributeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVCombatStateComponent> CombatStateComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVHitReactionComponent> HitReactionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVAttackTraceComponent> AttackTraceComponent;

    //--- Data ----------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Data")
    TObjectPtr<URVCharacterDataAsset> CharacterData;

    //--- Movement ------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "RV|Movement")
    FRotator AirRotationRate = FRotator(0.f, 0.f, 0.f);

private:
    FRotator OriginalRotationRate;
};
