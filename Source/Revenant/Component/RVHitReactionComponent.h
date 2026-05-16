#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/RVDamageable.h"
#include "RVHitReactionComponent.generated.h"

class ACharacter;
class URVAttributeComponent;
class URVCombatStateComponent;
class URVCombatDataAsset;
class URVCharacterDataAsset;
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
                        URVCombatDataAsset* InCombatData,
                        URVCharacterDataAsset* InCharacterData);

    // Called on weapon swap — updates reaction animations without re-initializing.
    void SetCombatData(URVCombatDataAsset* InCombatData) { CombatData = InCombatData; }

    // Snapshot set at stagger entry. URVAnimInstance polls this each frame.
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

    // Reaction animations source — player: from URVWeaponDataAsset, boss: from URVBossDataAsset.
    UPROPERTY()
    TObjectPtr<URVCombatDataAsset> CombatData;

    UPROPERTY()
    TObjectPtr<URVCharacterDataAsset> CharacterData;

    FTimerHandle StaggerHandle;
    float StaggerDirection = 0.f;

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