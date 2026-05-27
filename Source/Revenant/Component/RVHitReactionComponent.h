#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/RVDamageable.h"
#include "RVHitReactionComponent.generated.h"

class ARVCharacterBase;
class URVHitReactionAnimDataAsset;
class UAnimMontage;

UENUM(meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
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

    // Called by ARVCharacterBase::BeginPlay — passes data-driven float params
    // that come from CharacterData (owned by Base, not by this component).
    void InitParams(URVHitReactionAnimDataAsset* InHitReactionAnimData,
                    float InStaggerDuration,
                    float InStaggerThreshold,
                    float InKnockdownThreshold);

    void SetHitReactionAnimData(URVHitReactionAnimDataAsset* InData) { HitReactionAnimData = InData; }
    void SetHitReactCapability(ERVHitReactCapability InCap)          { HitReactCapability = InCap; }
    float GetStaggerDirection() const                                 { return StaggerDirection; }

    FRVOnGroggySequenceCompleted OnGroggySequenceCompleted;

protected:
    virtual void BeginPlay() override;

private:
    // Resolved in BeginPlay — valid for the component's entire lifetime.
    UPROPERTY()
    TObjectPtr<ARVCharacterBase> OwnerBase;

    UPROPERTY()
    TObjectPtr<URVHitReactionAnimDataAsset> HitReactionAnimData;

    FTimerHandle StaggerHandle;
    FTimerHandle GroggyTimerHandle;

    float StaggerDirection   = 0.f;
    float StaggerDuration    = 0.5f;
    float GroggyDuration     = 0.f;
    float StaggerThreshold   = 0.5f;
    float KnockdownThreshold = 0.4f;

    ERVHitReactCapability HitReactCapability =
        ERVHitReactCapability::Stagger | ERVHitReactCapability::Knockdown;

    bool CanHitReact(ERVHitReactCapability InCap) const
    {
        return (HitReactCapability & InCap) != ERVHitReactCapability::None;
    }

    void TriggerStagger(const FVector& InHitDirection);
    void TriggerKnockdown(const FVector& InHitDirection);

    // Convenience accessor — logs ensureMsgf on null AnimInstance.
    UAnimInstance* GetAnimInstance() const;

    UFUNCTION() void OnStaggerEnd();
    UFUNCTION() void OnStaggerMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
    UFUNCTION() void OnKnockdownMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
    UFUNCTION() void OnGetUpMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
    UFUNCTION() void OnGroggyStartMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
    UFUNCTION() void OnGroggyEndMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
};
