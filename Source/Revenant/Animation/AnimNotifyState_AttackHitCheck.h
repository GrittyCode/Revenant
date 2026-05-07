// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class URVCombatComponent;

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_AttackHitCheck.generated.h"

/**
 * 
 */
UCLASS()
class REVENANT_API UAnimNotifyState_AttackHitCheck : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	/** Opens hit window — clears HitActors on CombatComponent. */
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	/** Performs capsule sweep every tick within the hit window. */
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	
	/** Closes hit window — clears HitActors on CombatComponent. */
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
	virtual FString GetNotifyName_Implementation() const override;
	
	
private:
	UPROPERTY()
	TMap<USkeletalMeshComponent*, URVCombatComponent*> CachedCombatComps;

};
