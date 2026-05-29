#include "Animation/NotifyState/AnimNotifyState_ComboWindow.h"
#include "Character/Player/RVCharacterPlayer.h"

void UAnimNotifyState_ComboWindow::NotifyBegin(USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation, float TotalDuration,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    ARVCharacterPlayer* Player = Cast<ARVCharacterPlayer>(MeshComp->GetOwner());
    ensureMsgf(IsValid(Player),
        TEXT("[AnimNotifyState_ComboWindow] Owner is not ARVCharacterPlayer — check montage assignment"));
    if (!IsValid(Player)) { return; }

    Player->OpenComboWindow();
}

void UAnimNotifyState_ComboWindow::NotifyEnd(USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    ARVCharacterPlayer* Player = Cast<ARVCharacterPlayer>(MeshComp->GetOwner());
    if (!IsValid(Player)) { return; }

    Player->CloseComboWindow();
}

FString UAnimNotifyState_ComboWindow::GetNotifyName_Implementation() const
{
    return FString(TEXT("ComboWindow"));
}
