#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/RVDamageable.h"
#include "Component/Attribute/RVVitalComponent.h"
#include "Component/Combat/RVCombatStateComponent.h"
#include "RVCharacterBase.generated.h"

class URVHitReactionComponent;
class URVHitReactionAnimDataAsset;
class UMeshComponent;

// Fired once per confirmed hit
DECLARE_MULTICAST_DELEGATE_OneParam(FRVOnHitConfirmed, FVector /*ImpactLocation*/);

UCLASS(Abstract)
class REVENANT_API ARVCharacterBase : public ACharacter, public IRVDamageable
{
    GENERATED_BODY()

public:
    ARVCharacterBase();

    virtual bool ApplyDamage(const FRVHitInfo& InHitInfo) override;

    float GetHealthRatio() const;

    FRVOnHealthChanged& GetOnHealthChanged();
    FRVOnDeath&         GetOnDeath();
    FRVOnPoiseDepleted& GetOnPoiseDepleted();
    FRVOnPoiseChanged&  GetOnPoiseChanged();

    //--- Hit window (called by AnimNotifyState_AttackHitCheck) ---------------

    void OpenAttackHitWindow();
    void CloseAttackHitWindow();
    void ActivateHitCheck();

    //--- Weapon trail (overridden by Player and Boss) ------------------------

    virtual void ActivateWeaponTrail()   {}
    virtual void DeactivateWeaponTrail() {}

    //--- AnimInstance queries ------------------------------------------------

    float GetStaggerDirection() const;

    //--- Combat state operations ---------------------------------------------

    void AddCombatState   (ERVCombatState InState);
    void RemoveCombatState(ERVCombatState InState);
    bool HasCombatState   (ERVCombatState InState) const;
    bool CanAct(ERVCombatState InCoexistableStates = ERVCombatState::None) const;
    bool IsGrounded() const;

    void SetInvincible(bool bInvincible);
    bool IsInvincible() const;
    void ForceEndAllActions();

    //--- Poise ---------------------------------------------------------------

    float GetMaxPoise()   const;
    float GetPoiseRatio() const;
    void  ApplyPoiseDamage(float InAmount);
    void  ResetPoise();

    //--- Attack stat injection -----------------------------------------------

    // Player: OnWeaponChangedHandler. Boss: InitStats.
    void SetCombatStat(float InBaseDamage, float InBasePoiseDamage, float InAttackRadius);

    void TriggerStaggerWithMontage(UAnimMontage* InMontage);

    //--- Hit confirmed delegate ----------------------------------------------

    FRVOnHitConfirmed OnHitConfirmed;

protected:
    virtual void BeginPlay() override;
    virtual void Falling()   override;
    virtual void Landed(const FHitResult& Hit) override;
    virtual void OnDeath();
    virtual void InitStats() {}

    virtual URVHitReactionAnimDataAsset* GetHitReactionAnimData() const { return nullptr; }

    // Returns the mesh that owns WeaponRoot / WeaponTip sockets.
    virtual UMeshComponent* GetWeaponTraceMesh() const { return GetMesh(); }

    FVector GetForwardLocation(float InOffset = 1.f) const;
    FVector GetGroundOrigin() const;

    UPROPERTY(VisibleAnywhere, Category = "RV|Components")
    TObjectPtr<URVVitalComponent> VitalComponent;

    UPROPERTY(VisibleAnywhere, Category = "RV|Components")
    TObjectPtr<URVCombatStateComponent> CombatStateComponent;

    UPROPERTY(VisibleAnywhere, Category = "RV|Components")
    TObjectPtr<URVHitReactionComponent> HitReactionComponent;

    UPROPERTY(EditDefaultsOnly, Category = "RV|Movement")
    FRotator AirRotationRate = FRotator(0.f, 0.f, 0.f);

private:
    //--- Hit detection state -------------------------------------------------

    // Set once at BeginPlay via GetWeaponTraceMesh().
    UPROPERTY()
    TObjectPtr<UMeshComponent> WeaponTraceMesh;

    float CachedBaseDamage      = 0.f;
    float CachedBasePoiseDamage = 0.f;
    float CachedAttackRadius    = 40.f;

    // Per-swing dedup — cleared on OpenAttackHitWindow / CloseAttackHitWindow.
    UPROPERTY()
    TSet<TObjectPtr<AActor>> HitActors;

    void PerformHit();

    //--- Rotation ------------------------------------------------------------

    FRotator OriginalRotationRate;
};