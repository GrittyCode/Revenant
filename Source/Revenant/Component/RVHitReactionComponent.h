#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/RVDamageable.h"
#include "RVHitReactionComponent.generated.h"

class ACharacter;
class URVAttributeComponent;
class URVCombatStateComponent;
class URVEquipmentComponent;
class URVCharacterDataAsset;
class UAnimMontage;

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVHitReactionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVHitReactionComponent();

    void HandleHit(const FRVHitInfo& InHitInfo);

    // Guard break routes through here so recovery is montage-length-driven, not timer-driven.
    void TriggerStaggerWithMontage(UAnimMontage* InMontage);

    void InitReferences(ACharacter* InOwnerCharacter,
                        URVCombatStateComponent* InCombatStateComponent,
                        URVAttributeComponent* InAttributeComponent,
                        URVEquipmentComponent* InEquipmentComponent,
                        URVCharacterDataAsset* InCharacterData);

protected:
    virtual void BeginPlay() override;

private:
    //--- Cached References ---------------------------------------------------

    UPROPERTY()
    TObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<URVCombatStateComponent> CombatStateComponent;

    UPROPERTY()
    TObjectPtr<URVAttributeComponent> AttributeComponent;

    UPROPERTY()
    TObjectPtr<URVEquipmentComponent> EquipmentComponent;

    UPROPERTY()
    TObjectPtr<URVCharacterDataAsset> CharacterData;

    //--- State ---------------------------------------------------------------

    // Resets on Groggy entry so each Groggy cycle starts fresh.
    int32 StaggerCount = 0;

    FTimerHandle StaggerHandle;
    FTimerHandle GroggyHandle;

    //--- Reaction Triggers ---------------------------------------------------

    void TriggerPhysicalReaction(const FVector& InHitDirection);
    void TriggerStagger(const FVector& InHitDirection);
    void TriggerGroggy();
    void TriggerKnockdown();

    //--- Callbacks -----------------------------------------------------------

    UFUNCTION()
    void OnStaggerEnd();

    UFUNCTION()
    void OnStaggerMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void EndGroggy();

    UFUNCTION()
    void OnKnockdownMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void OnGetUpMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
};