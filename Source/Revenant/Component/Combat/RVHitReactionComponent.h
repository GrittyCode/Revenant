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

UCLASS(ClassGroup=(Revenant))
class REVENANT_API URVHitReactionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URVHitReactionComponent();
	virtual void BeginPlay() override;

	void HandleHit(const FRVHitInfo& InHitInfo);
	void TriggerStaggerWithMontage(UAnimMontage* InMontage);
	void TriggerGroggy(float InGroggyDuration);
	void EndGroggy();
	void AbortGroggy();

	void InitParams(URVHitReactionAnimDataAsset* InHitReactionAnimData,
	                float InStaggerDuration,
	                float InStaggerThreshold,
	                float InKnockdownThreshold);

	void SetHitReactionAnimData(URVHitReactionAnimDataAsset* InData) { HitReactionAnimData = InData; }
	void SetHitReactCapability(ERVHitReactCapability InCap) { HitReactCapability = InCap; }
	float GetStaggerDirection() const { return StaggerDirection; }

	FRVOnGroggySequenceCompleted OnGroggySequenceCompleted;

private:
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

	UAnimInstance* GetAnimInstance() const;

	void TriggerStagger(const FVector& InHitDirection);
	void TriggerKnockdown(const FVector& InHitDirection);

	void OnStaggerEnd();
	void OnStaggerMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	void OnKnockdownMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	void OnGetUpMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	void OnGroggyStartMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	void OnGroggyEndMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
};
