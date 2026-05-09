#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "RVHeavyAttackComponent.generated.h"

class ACharacter;
class URVAttributeComponent;
class URVEquipmentComponent;
class URVCombatStateComponent;
class UAnimMontage;

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVHeavyAttackComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVHeavyAttackComponent();

    /**
     * Begins heavy attack charge. Plays HeavyChargeMontage (loops until released).
     * Consumes stamina immediately — charge cancellation is a penalty.
     */
    void StartHeavyAttack();

    /**
     * Requests heavy attack release.
     * Buffered in bPendingRelease if Loop section not yet reached.
     */
    void ReleaseHeavyAttack();

    /**
     * Called by AnimNotify_HeavyAttackReady on Loop section entry.
     * Activates release gating and flushes buffered release.
     */
    void SetHeavyAttackReady(bool bReady);

    /** Called via CombatStateComponent::OnForceEnd. Stops montage and resets state. */
    void ForceEndHeavyAttack();

    void InitReferences(ACharacter* InOwnerCharacter,
                        URVCombatStateComponent* InCombatStateComponent,
                        URVAttributeComponent* InAttributeComponent,
                        URVEquipmentComponent* InEquipmentComponent);

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<URVCombatStateComponent> CombatStateComponent;

    UPROPERTY()
    TObjectPtr<URVAttributeComponent> AttributeComponent;

    UPROPERTY()
    TObjectPtr<URVEquipmentComponent> EquipmentComponent;

    UPROPERTY(EditDefaultsOnly, Category = "RV|HeavyAttack")
    float MaxChargeTime = 1.5f;

    FTimerHandle ChargeAutoReleaseHandle;

    bool bCanHeavyRelease = false;
    bool bPendingRelease  = false;
    bool bIsAutoRelease   = false;

    void EndHeavyAttack();
    void ExecuteHeavyAttack();

    UFUNCTION()
    void OnChargeAutoRelease();

    void OnChargeMontageBlendingOut(UAnimMontage*, bool bInterrupted);
    void OnReleaseMontageBlendingOut(UAnimMontage*, bool bInterrupted);
};