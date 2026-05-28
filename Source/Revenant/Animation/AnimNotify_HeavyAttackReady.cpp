#include "Animation/AnimNotify_HeavyAttackReady.h"
#include "Character/Player/RVCharacterPlayer.h"

void UAnimNotify_HeavyAttackReady::Notify(USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    ARVCharacterPlayer* Player = Cast<ARVCharacterPlayer>(MeshComp->GetOwner());
    ensureMsgf(IsValid(Player),
        TEXT("[AnimNotify_HeavyAttackReady] Owner is not ARVCharacterPlayer — check montage assignment"));
    if (!IsValid(Player)) { return; }

    Player->SetHeavyAttackReady(true);
}

FString UAnimNotify_HeavyAttackReady::GetNotifyName_Implementation() const
{
    return FString(TEXT("HeavyAttackReady"));
}
