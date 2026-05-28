#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/RVHitCheckTarget.h"
#include "Interface/RVDamageable.h"
#include "Component/RVVitalComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "RVCharacterBase.generated.h"

class URVHitReactionComponent;
class URVAttackTraceComponent;
class URVHitReactionAnimDataAsset;
class UMeshComponent;
class UNiagaraSystem;
class UParticleSystem;
class USoundBase;

UCLASS(Abstract)
class REVENANT_API ARVCharacterBase : public ACharacter, public IRVHitCheckTarget, public IRVDamageable
{
    GENERATED_BODY()

public:
    ARVCharacterBase();

    virtual void ActivateHitCheck() override;
    virtual bool ApplyDamage(const FRVHitInfo& InHitInfo) override;

    //--- Health queries (external: GameMode, Widget) -------------------------

    UFUNCTION(BlueprintCallable, Category = "RV|Vital")
    float GetHealthRatio() const;

    FRVOnHealthChanged& GetOnHealthChanged();
    FRVOnDeath&         GetOnDeath();
    FRVOnPoiseDepleted& GetOnPoiseDepleted();
    FRVOnPoiseChanged&  GetOnPoiseChanged();

    //--- AnimNotify entry points ---------------------------------------------

    void OpenAttackHitWindow();
    void CloseAttackHitWindow();

    virtual void ActivateWeaponTrail()   {}
    virtual void DeactivateWeaponTrail() {}

    //--- AnimInstance state queries ------------------------------------------

    bool  IsInCombatState(ERVCombatState InState) const;
    float GetStaggerDirection() const;

    //--- Combat state operations ---------------------------------------------

    void AddCombatState(ERVCombatState InState);
    void RemoveCombatState(ERVCombatState InState);
    bool HasCombatState(ERVCombatState InState) const;
    bool CanAct(ERVCombatState InCoexistableStates = ERVCombatState::None) const;
    bool IsGrounded() const;
    void SetInvincible(bool bInvincible);
    bool IsInvincible() const;
    void ForceEndAllActions();

    //--- Poise operations ----------------------------------------------------

    float GetMaxPoise()   const;
    float GetPoiseRatio() const;
    void  ApplyPoiseDamage(float InAmount);
    void  ResetPoise();

    //--- Attack trace operations ---------------------------------------------

    void SetCombatStat(float InBaseDamage, float InBasePoiseDamage, float InAttackRadius);
    void SetHitFX(UNiagaraSystem* InNiagara, UParticleSystem* InCascade, USoundBase* InSFX);

    //--- Hit reaction operations ---------------------------------------------

    void TriggerStaggerWithMontage(UAnimMontage* InMontage);

protected:
    virtual void BeginPlay() override;
    virtual void Falling()   override;
    virtual void Landed(const FHitResult& Hit) override;

    // Final classes own their DataAsset and are responsible for all stat init,
    // including VitalComponent::InitFromStatRow and HitReactionComponent::InitParams.
    virtual void InitStats() {}

    virtual URVHitReactionAnimDataAsset* GetHitReactionAnimData() const { return nullptr; }

    UFUNCTION()
    virtual void OnDeath();

    virtual UMeshComponent* GetWeaponTraceMesh() const { return GetMesh(); }

    FVector GetForwardLocation(float InOffset = 1.f) const;
    FVector GetGroundOrigin() const;

    //--- Components ----------------------------------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVVitalComponent> VitalComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVCombatStateComponent> CombatStateComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVHitReactionComponent> HitReactionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVAttackTraceComponent> AttackTraceComponent;

    //--- Movement ------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "RV|Movement")
    FRotator AirRotationRate = FRotator(0.f, 0.f, 0.f);

private:
    FRotator OriginalRotationRate;
};