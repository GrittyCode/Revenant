#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RVSevarogAnimInstance.generated.h"

class ARVSevarogCharacter;
class UBlendSpace;

// [설계-2] AnimInstance는 캐릭터 파사드만 통해 상태를 읽는다.
// URVCombatStateComponent, URVHitReactionComponent 직접 참조 제거.
UCLASS()
class REVENANT_API URVSevarogAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
    //--- Locomotion ----------------------------------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    float Speed = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UBlendSpace> CachedLocomotionBS;

    //--- State ---------------------------------------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    uint8 bIsAttacking : 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    uint8 bIsInHitReaction : 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    uint8 bIsKnockedDown : 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    uint8 bIsGroggy : 1;

    //--- Hit Reaction --------------------------------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    float StaggerDirection = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Animation")
    TObjectPtr<UBlendSpace> CachedStaggerBlendSpace;

private:
    UPROPERTY()
    TObjectPtr<ARVSevarogCharacter> OwnerSevarog;
};
