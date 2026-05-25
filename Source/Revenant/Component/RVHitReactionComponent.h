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

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ERVHitReactCapability : uint8
{
    None      = 0,
    Stagger   = 1 << 0,
    Knockdown = 1 << 1,
    Groggy    = 1 << 2,
};
ENUM_CLASS_FLAGS(ERVHitReactCapability)

DECLARE_MULTICAST_DELEGATE(FRVOnGroggySequenceCompleted);

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVHitReactionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVHitReactionComponent();

    void HandleHit(const FRVHitInfo& InHitInfo);

    void TriggerStaggerWithMontage(UAnimMontage* InMontage);
    void TriggerGroggy(float InGroggyDuration);
    void EndGroggy();
    void AbortGroggy();

    void InitReferences(ACharacter* InOwnerCharacter,
                        URVCombatStateComponent* InCombatStateComponent,
                        URVAttributeComponent* InAttributeComponent,
                        URVHitReactionAnimDataAsset* InHitReactionAnimData,
                        float InStaggerDuration,
                        float InStaggerThreshold,
                        float InKnockdownThreshold);

    void SetHitReactionAnimData(URVHitReactionAnimDataAsset* InHitReactionAnimData) { HitReactionAnimData = InHitReactionAnimData; }
    void SetStaggerDuration(float InDuration)               { StaggerDuration = InDuration; }
    void SetHitReactCapability(ERVHitReactCapability InCap) { HitReactCapability = InCap; }
    float GetStaggerDirection() const                       { return StaggerDirection; }

    FRVOnGroggySequenceCompleted OnGroggySequenceCompleted;

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
    FTimerHandle GroggyTimerHandle;

    float StaggerDirection   = 0.f;
    float StaggerDuration    = 0.5f;
    float GroggyDuration     = 0.f;

    // Ratio thresholds derived from FRVCharacterStatRow.
    float StaggerThreshold   = 0.5f;
    float KnockdownThreshold = 0.4f;

    ERVHitReactCapability HitReactCapability = ERVHitReactCapability::Stagger | ERVHitReactCapability::Knockdown;

    bool CanHitReact(ERVHitReactCapability InCapability) const { return (HitReactCapability & InCapability) != ERVHitReactCapability::None; }

    void TriggerStagger(const FVector& InHitDirection);
    void TriggerKnockdown(const FVector& InHitDirection);

    UFUNCTION() void OnStaggerEnd();
    UFUNCTION() void OnStaggerMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
    UFUNCTION() void OnKnockdownMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
    UFUNCTION() void OnGetUpMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
    UFUNCTION() void OnGroggyStartMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
    UFUNCTION() void OnGroggyEndMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
};