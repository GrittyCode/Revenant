#include "Animation/RVSevarogAnimInstance.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "Data/RVSevarogDataAsset.h"
#include "Data/RVHitReactionAnimDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"

void URVSevarogAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerSevarog = Cast<ARVSevarogCharacter>(GetOwningActor());
    if (!IsValid(OwnerSevarog)) { return; }

    // [설계-2] 컴포넌트 직접 캐시 제거 — 파사드 메서드로 대체.
    const URVSevarogDataAsset* Data = OwnerSevarog->GetSevarogData();
    if (!IsValid(Data)) { return; }

    CachedLocomotionBS = Data->LocomotionBS;

    if (IsValid(Data->HitReactionAnimData))
    {
        CachedStaggerBlendSpace = Data->HitReactionAnimData->StaggerBlendSpace;
    }
}

void URVSevarogAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!IsValid(OwnerSevarog)) { return; }

    // [이디엄-5] Velocity.Size() → Size2D(): 수직 속도를 제외해야 지면 로코모션 블렌드가 정확.
    // 낙하 중에 Size()를 쓰면 실제보다 높은 속도 값이 되어 RunBS로 잘못 블렌딩됨.
    Speed = OwnerSevarog->GetCharacterMovement()->Velocity.Size2D();

    // [설계-2] 컴포넌트 직접 접근 대신 ARVSevarogCharacter 파사드 메서드 사용.
    bIsAttacking     = OwnerSevarog->IsAttacking();
    bIsInHitReaction = OwnerSevarog->IsInHitReaction();
    bIsKnockedDown   = OwnerSevarog->IsKnockedDown();
    bIsGroggy        = OwnerSevarog->IsGroggy();

    StaggerDirection = OwnerSevarog->GetStaggerDirectionForAnim();
}
