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
UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVAttackTraceComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVAttackTraceComponent();
    virtual void BeginPlay() override;

    // Called by ARVCharacterBase::BeginPlay — passes the trace-origin mesh
    void InitTraceMesh(UMeshComponent* InTraceMesh);

    // Injects attack base stats. Player: called from OnWeaponChangedHandler.
    void SetCombatStat(float InBaseDamage, float InBasePoiseDamage, float InAttackRadius);

    void SetHitFX(UNiagaraSystem* InNiagara, UParticleSystem* InCascade, USoundBase* InSFX);

    // Called by AnimNotifyState_AttackHitCheck::NotifyBegin — clears already-hit set.
    void OpenHitWindow();

    // Called by AnimNotifyState_AttackHitCheck::NotifyEnd and CombatStateComponent::OnForceEnd.
    void CloseHitWindow();

    // Called per-tick by AnimNotifyState_AttackHitCheck::NotifyTick.
    // Reads current montage stat multipliers, performs capsule overlap, applies damage and FX.
    void PerformAttackTrace();

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
    UPROPERTY()
    TSet<TObjectPtr<AActor>> HitActors;
};
