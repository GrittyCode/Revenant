// Source/Revenant/Component/RVCombatStateComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVCombatStateComponent.generated.h"

class ACharacter;
class UMeshComponent;
class UCharacterMovementComponent;

UENUM(meta=(Bitflags))
enum class ERVCombatState : uint16
{
    None           = 0,
    Attacking      = 1 << 0,
    HeavyAttacking = 1 << 1,
    Dodging        = 1 << 2,
    Guarding       = 1 << 3,
    HeavyCharging  = 1 << 4,
    HitReaction    = 1 << 5,
    Groggy         = 1 << 6,
    Knockdown      = 1 << 7,
};
ENUM_CLASS_FLAGS(ERVCombatState);

DECLARE_MULTICAST_DELEGATE_OneParam(FRVOnCombatStateChanged, ERVCombatState);
DECLARE_MULTICAST_DELEGATE(FRVOnForceEnd);

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVCombatStateComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVCombatStateComponent();

    //--- Delegates -----------------------------------------------------------

    FRVOnCombatStateChanged OnStateChanged;
    FRVOnForceEnd           OnForceEnd;

    //--- Attack Trace --------------------------------------------------------

    void OpenHitWindow();
    void CloseHitWindow();
    void PerformAttackTrace();

    //--- State Control -------------------------------------------------------

    void ForceEndAllActions();

    //--- State Queries -------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "RV|Combat")
    bool IsInState(ERVCombatState InState) const { return (CurrentStates & InState) != ERVCombatState::None; }

    UFUNCTION(BlueprintCallable, Category = "RV|Combat")
    bool IsInvincible() const { return bIsInvincible; }

    UFUNCTION(BlueprintCallable, Category = "RV|Combat")
    bool IsGrounded() const;

    ERVCombatState GetActiveStates() const { return CurrentStates; }

    bool CheckAvailableState(ERVCombatState InCoexistableStates = ERVCombatState::None) const;

    //--- State Mutators ------------------------------------------------------

    FORCEINLINE void AddState(ERVCombatState InState) { CurrentStates |= InState; OnStateChanged.Broadcast(CurrentStates); }
    FORCEINLINE void RemoveState(ERVCombatState InState) { CurrentStates &= ~InState; OnStateChanged.Broadcast(CurrentStates); }
    FORCEINLINE bool HasState(ERVCombatState InState) const { return (CurrentStates & InState) != ERVCombatState::None; }

    void SetInvincible(bool bInInvincible) { bIsInvincible = bInInvincible; }

    //--- Reference Injection -------------------------------------------------

    void InitReferences(ACharacter* InOwnerCharacter,
                        UMeshComponent* InTraceMesh,
                        UCharacterMovementComponent* InMovementComponent);

    /**
     * Injects attack base stats from the current weapon (player) or enemy stat row (boss).
     * Called by ARVCharacterPlayer::OnWeaponChangedHandler and ARVBossCharacter::BeginPlay.
     */
    void SetCombatStat(float InBaseDamage, float InBasePoiseDamage, float InAttackRadius);

    //--- Attack State Handlers -----------------------------------------------

    void OnAttackStarted();
    void OnAttackEnded();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<UMeshComponent> TraceMesh;

    UPROPERTY()
    TObjectPtr<UCharacterMovementComponent> MovementComponent;

    ERVCombatState CurrentStates = ERVCombatState::None;
    bool           bIsInvincible = false;

    // Cached attack base stats. Set via SetCombatStat().
    // Player: updated on weapon swap. Boss: set once in BeginPlay.
    float CachedBaseDamage      = 0.f;
    float CachedBasePoiseDamage = 0.f;
    float CachedAttackRadius    = 40.f;

    TSet<TWeakObjectPtr<AActor>> HitActors;
};