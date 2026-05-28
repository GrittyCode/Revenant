#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVAttackTraceComponent.generated.h"

class ACharacter;
class UMeshComponent;
class UNiagaraSystem;
class UParticleSystem;
class USoundBase;

// Performs per-tick weapon hit detection and applies damage.
// Extracted from URVCombatStateComponent to enforce single responsibility.
// Used by both Player (weapon swing) and Boss (melee combo chain via AnimNotifyState_AttackHitCheck).
// SoulSiphon / Subjugation use ApplyRadialDamageAt — not this component.
UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVAttackTraceComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVAttackTraceComponent();

    // Called by ARVCharacterBase::BeginPlay — passes the trace-origin mesh
    // (weapon mesh for player, skeletal mesh for boss).
    void InitTraceMesh(UMeshComponent* InTraceMesh);

    // Injects attack base stats. Player: called from OnWeaponChangedHandler.
    // Boss: called from InitStats. Both route through ARVCharacterBase::SetCombatStat().
    void SetCombatStat(float InBaseDamage, float InBasePoiseDamage, float InAttackRadius);

    // Injects hit impact VFX / SFX. Pass nullptr for unused slots.
    void SetHitFX(UNiagaraSystem* InNiagara, UParticleSystem* InCascade, USoundBase* InSFX);

    // Called by AnimNotifyState_AttackHitCheck::NotifyBegin — clears already-hit set.
    void OpenHitWindow();

    // Called by AnimNotifyState_AttackHitCheck::NotifyEnd and CombatStateComponent::OnForceEnd.
    void CloseHitWindow();

    // Called per-tick by AnimNotifyState_AttackHitCheck::NotifyTick.
    // Reads current montage stat multipliers, performs capsule overlap, applies damage and FX.
    void PerformAttackTrace();

protected:
    virtual void BeginPlay() override;

private:
    // Resolved in BeginPlay via GetOwner(). Valid for the component's entire lifetime.
    UPROPERTY()
    TObjectPtr<ACharacter> OwnerCharacter;

    // Set by ARVCharacterBase::BeginPlay after components self-initialize.
    UPROPERTY()
    TObjectPtr<UMeshComponent> TraceMesh;

    float CachedBaseDamage      = 0.f;
    float CachedBasePoiseDamage = 0.f;
    float CachedAttackRadius    = 40.f;

    UPROPERTY()
    TObjectPtr<UNiagaraSystem>  HitImpactEffect;

    UPROPERTY()
    TObjectPtr<UParticleSystem> HitImpactEffectCascade;

    UPROPERTY()
    TObjectPtr<USoundBase> HitSFX;

    // Actors already hit in the current window — prevents multi-hit per swing.
    // Entries are live actors (cleared each swing), so strong refs are correct here.
    UPROPERTY()
    TSet<TObjectPtr<AActor>> HitActors;
};
