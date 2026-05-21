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

// fired when the full groggy montage sequence (Start → Loop → End) completes naturally
DECLARE_MULTICAST_DELEGATE(FRVOnGroggySequenceCompleted);

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVHitReactionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVHitReactionComponent();

    void HandleHit(const FRVHitInfo& InHitInfo);

    /** Guard break routes through here so recovery is montage-length-driven. */
    void TriggerStaggerWithMontage(UAnimMontage* InMontage);

    /** Starts the 3-stage groggy sequence (Start → Loop → End). Fires OnGroggySequenceCompleted on completion. */
    void TriggerGroggy(float InGroggyDuration);

    /** Transitions from Loop to End montage. Called when groggy duration elapses or forced externally. */
    void EndGroggy();

    /** Stops groggy timer and montage immediately without firing OnGroggySequenceCompleted. Used on death. */
    void AbortGroggy();

    void InitReferences(ACharacter* InOwnerCharacter,
                        URVCombatStateComponent* InCombatStateComponent,
                        URVAttributeComponent* InAttributeComponent,
                        URVHitReactionAnimDataAsset* InHitReactionAnimData,
                        float InStaggerDuration);

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

    FTimerHandle          StaggerHandle;
    FTimerHandle          GroggyTimerHandle;
	
    float                 StaggerDirection   = 0.f;
    float                 StaggerDuration    = 0.5f;
	float                 GroggyDuration     = 0.f;
	
    // all characters support stagger and knockdown by default; override per character type in BeginPlay
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