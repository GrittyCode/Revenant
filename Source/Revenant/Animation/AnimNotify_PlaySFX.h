#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_PlaySFX.generated.h"

class USoundBase;

// SFX-only notify. Use when audio is needed without any accompanying VFX,
// e.g. boss voice lines, groggy grunts, or intro dialogue placed directly on montage tracks.
// When SFX accompanies a VFX, assign it to FRVFXEntry::SFX on AnimNotify_SpawnFX instead.
UCLASS(meta = (DisplayName = "Play SFX"))
class REVENANT_API UAnimNotify_PlaySFX : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override { return TEXT("PlaySFX"); }

	UPROPERTY(EditAnywhere, Category = "RV|SFX")
	TObjectPtr<USoundBase> SFX;

	// Socket for 3D placement. NAME_None uses mesh component location.
	UPROPERTY(EditAnywhere, Category = "RV|SFX")
	FName SocketName = NAME_None;

	UPROPERTY(EditAnywhere, Category = "RV|SFX", meta = (ClampMin = "0.0"))
	float VolumeMultiplier = 1.f;
};