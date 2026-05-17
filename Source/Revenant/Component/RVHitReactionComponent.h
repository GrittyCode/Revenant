#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/RVDamageable.h"
#include "RVHitReactionComponent.generated.h"

class ACharacter;
class URVAttributeComponent;
class URVCombatStateComponent;
class URVHitReactionAnimDataAsset;
class UAnimMontage;

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVHitReactionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVHitReactionComponent();

    void HandleHit(const FRVHitInfo& InHitInfo);

    /** Guard break routes through here so recovery is montage-length-driven. */
    void TriggerStaggerWithMontage(UAnimMontage* InMontage);

    void InitReferences(ACharacter* InOwnerCharacter,
                        URVCombatStateComponent* InCombatStateComponent,
                        URVAttributeComponent* InAttributeComponent,
                        URVHitReactionAnimDataAsset* InHitReactionAnimData,
                        float InStaggerDuration);

    void SetHitReactionAnimData(URVHitReactionAnimDataAsset* InHitReactionAnimData) { HitReactionAnimData = InHitReactionAnimData; }
    void SetStaggerDuration(float InDuration) { StaggerDuration = InDuration; }
    float GetStaggerDirection() const { return StaggerDirection; }

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
    TObjectPtr<URVHitReactionAnimDataAsset> HitReactionAnimData;

    FTimerHandle StaggerHandle;
    float StaggerDirection = 0.f;
    float StaggerDuration  = 0.5f;

    void TriggerStagger(const FVector& InHitDirection);
    void TriggerKnockdown(const FVector& InHitDirection);

    UFUNCTION()
    void OnStaggerEnd();

    UFUNCTION()
    void OnStaggerMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void OnKnockdownMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void OnGetUpMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
};