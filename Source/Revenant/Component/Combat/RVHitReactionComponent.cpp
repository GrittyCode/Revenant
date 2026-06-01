#include "Component/Combat/RVHitReactionComponent.h"
#include "Character/Base/RVCharacterBase.h"
#include "Data/Asset/RVHitReactionAnimDataAsset.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "KismetAnimationLibrary.h"

URVHitReactionComponent::URVHitReactionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URVHitReactionComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerBase = Cast<ARVCharacterBase>(GetOwner());
    if (!ensureMsgf(IsValid(OwnerBase),
        TEXT("[URVHitReactionComponent] Owner must be ARVCharacterBase"))) { return; }
}

void URVHitReactionComponent::InitParams(
    URVHitReactionAnimDataAsset* InHitReactionAnimData,
    float InStaggerDuration, float InStaggerThreshold, float InKnockdownThreshold)
{
    if (!ensureMsgf(IsValid(InHitReactionAnimData),
        TEXT("[%s] InitParams: HitReactionAnimData not assigned"), *GetNameSafe(OwnerBase))) { return; }

    HitReactionAnimData  = InHitReactionAnimData;
    StaggerDuration      = InStaggerDuration;
    StaggerThreshold     = InStaggerThreshold;
    KnockdownThreshold   = InKnockdownThreshold;
}

UAnimInstance* URVHitReactionComponent::GetAnimInstance() const
{
    UAnimInstance* AnimInst = OwnerBase->GetMesh()->GetAnimInstance();
    ensureMsgf(IsValid(AnimInst),
        TEXT("[%s] URVHitReactionComponent: AnimInstance missing — check ABP assignment"),
        *GetNameSafe(OwnerBase));
    return AnimInst;
}

//--- Main Entry Point --------------------------------------------------------

void URVHitReactionComponent::HandleHit(const FRVHitInfo& InHitInfo)
{
    if (OwnerBase->HasCombatState(ERVCombatState::Knockdown)) { return; }

    const float MaxPoise = OwnerBase->GetMaxPoise();

    const bool bSingleHitKnockdown = MaxPoise > 0.f
        && (InHitInfo.PoiseDamage / MaxPoise) >= KnockdownThreshold;

    OwnerBase->ApplyPoiseDamage(InHitInfo.PoiseDamage);

    const float PoiseRatio = OwnerBase->GetPoiseRatio();

    const bool bShouldKnockdown = bSingleHitKnockdown
        || !OwnerBase->IsGrounded()
        || OwnerBase->HasCombatState(ERVCombatState::HitReaction);

    const bool bReactionNeeded = bShouldKnockdown || (PoiseRatio <= StaggerThreshold);
    if (!bReactionNeeded) { return; }

    const ERVHitReactCapability Required = bShouldKnockdown
        ? ERVHitReactCapability::Knockdown
        : ERVHitReactCapability::Stagger;

    if (!CanHitReact(Required)) { return; }

    OwnerBase->ForceEndAllActions();
    OwnerBase->ResetPoise();

    if (bShouldKnockdown)
    {
        if (OwnerBase->HasCombatState(ERVCombatState::HitReaction))
        {
            GetWorld()->GetTimerManager().ClearTimer(StaggerHandle);
            OwnerBase->RemoveCombatState(ERVCombatState::HitReaction);
        }
        TriggerKnockdown(InHitInfo.HitDirection);
    }
    else
    {
        TriggerStagger(InHitInfo.HitDirection);
    }
}

//--- Guard-Break Entry Point -------------------------------------------------

void URVHitReactionComponent::TriggerStaggerWithMontage(UAnimMontage* InMontage)
{
    if (!IsValid(InMontage)) { return; }

    UAnimInstance* AnimInst = GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    OwnerBase->AddCombatState(ERVCombatState::HitReaction);
    AnimInst->Montage_Play(InMontage);

    FOnMontageBlendingOutStarted BlendingOutDelegate;
    BlendingOutDelegate.BindUObject(this, &URVHitReactionComponent::OnStaggerMontageBlendingOut);
    AnimInst->Montage_SetBlendingOutDelegate(BlendingOutDelegate, InMontage);
}

//--- Groggy ------------------------------------------------------------------

void URVHitReactionComponent::TriggerGroggy(float InGroggyDuration)
{
    if (!ensureMsgf(IsValid(HitReactionAnimData),
        TEXT("[%s] TriggerGroggy: HitReactionAnimData not assigned"),
        *GetNameSafe(OwnerBase))) { return; }

    UAnimInstance* AnimInst = GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    GroggyDuration = InGroggyDuration;

    UAnimMontage* StartMontage = HitReactionAnimData->GroggyStunStartMontage;
    if (!IsValid(StartMontage))
    {
        UAnimMontage* LoopMontage = HitReactionAnimData->GroggyStunLoopMontage;
        if (IsValid(LoopMontage)) { AnimInst->Montage_Play(LoopMontage); }
        GetWorld()->GetTimerManager().SetTimer(
            GroggyTimerHandle, this, &URVHitReactionComponent::EndGroggy, GroggyDuration, false);
        return;
    }

    AnimInst->Montage_Play(StartMontage);

    FOnMontageBlendingOutStarted BlendOutDelegate;
    BlendOutDelegate.BindUObject(this, &URVHitReactionComponent::OnGroggyStartMontageBlendingOut);
    AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, StartMontage);
}

void URVHitReactionComponent::EndGroggy()
{
    GetWorld()->GetTimerManager().ClearTimer(GroggyTimerHandle);

    if (!ensureMsgf(IsValid(HitReactionAnimData),
        TEXT("[%s] EndGroggy: HitReactionAnimData not assigned"),
        *GetNameSafe(OwnerBase))) { return; }

    UAnimInstance* AnimInst = GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    UAnimMontage* LoopMontage = HitReactionAnimData->GroggyStunLoopMontage;
    if (IsValid(LoopMontage)) { AnimInst->Montage_Stop(0.2f, LoopMontage); }

    UAnimMontage* EndMontage = HitReactionAnimData->GroggyStunEndMontage;
    if (!IsValid(EndMontage)) { OnGroggySequenceCompleted.Broadcast(); return; }

    AnimInst->Montage_Play(EndMontage);

    FOnMontageBlendingOutStarted BlendOutDelegate;
    BlendOutDelegate.BindUObject(this, &URVHitReactionComponent::OnGroggyEndMontageBlendingOut);
    AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, EndMontage);
}

void URVHitReactionComponent::AbortGroggy()
{
    GetWorld()->GetTimerManager().ClearTimer(GroggyTimerHandle);
}

//--- Reaction Triggers -------------------------------------------------------

void URVHitReactionComponent::TriggerStagger(const FVector& InHitDirection)
{
    StaggerDirection = UKismetAnimationLibrary::CalculateDirection(
        InHitDirection, OwnerBase->GetActorRotation());

    OwnerBase->AddCombatState(ERVCombatState::HitReaction);

    GetWorld()->GetTimerManager().SetTimer(
        StaggerHandle, this, &URVHitReactionComponent::OnStaggerEnd, StaggerDuration, false);
}

void URVHitReactionComponent::TriggerKnockdown(const FVector& InHitDirection)
{
    UAnimMontage* KnockdownMontage = IsValid(HitReactionAnimData)
        ? HitReactionAnimData->KnockdownMontage : nullptr;
    if (!IsValid(KnockdownMontage)) { return; }

    UAnimInstance* AnimInst = GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    OwnerBase->SetActorRotation(FRotationMatrix::MakeFromX(InHitDirection).Rotator());
    OwnerBase->AddCombatState(ERVCombatState::Knockdown);

    AnimInst->Montage_Play(KnockdownMontage);

    FOnMontageBlendingOutStarted BlendingOutDelegate;
    BlendingOutDelegate.BindUObject(this, &URVHitReactionComponent::OnKnockdownMontageBlendingOut);
    AnimInst->Montage_SetBlendingOutDelegate(BlendingOutDelegate, KnockdownMontage);
}

//--- Callbacks ---------------------------------------------------------------

void URVHitReactionComponent::OnStaggerEnd()
{
    OwnerBase->RemoveCombatState(ERVCombatState::HitReaction);
}

void URVHitReactionComponent::OnStaggerMontageBlendingOut(UAnimMontage*, bool)
{
    OwnerBase->RemoveCombatState(ERVCombatState::HitReaction);
}

void URVHitReactionComponent::OnKnockdownMontageBlendingOut(UAnimMontage*, bool)
{
    UAnimMontage* GetUpMontage = IsValid(HitReactionAnimData)
        ? HitReactionAnimData->GetUpMontage : nullptr;
    if (!IsValid(GetUpMontage)) { return; }

    UAnimInstance* AnimInst = GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    AnimInst->Montage_Play(GetUpMontage);

    FOnMontageBlendingOutStarted GetUpDelegate;
    GetUpDelegate.BindUObject(this, &URVHitReactionComponent::OnGetUpMontageBlendingOut);
    AnimInst->Montage_SetBlendingOutDelegate(GetUpDelegate, GetUpMontage);
}

void URVHitReactionComponent::OnGetUpMontageBlendingOut(UAnimMontage*, bool)
{
    OwnerBase->RemoveCombatState(ERVCombatState::Knockdown);
}

void URVHitReactionComponent::OnGroggyStartMontageBlendingOut(UAnimMontage*, bool bInterrupted)
{
    if (bInterrupted) { return; }

    UAnimMontage* LoopMontage = IsValid(HitReactionAnimData)
        ? HitReactionAnimData->GroggyStunLoopMontage : nullptr;
    if (!IsValid(LoopMontage)) { return; }

    UAnimInstance* AnimInst = GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    AnimInst->Montage_Play(LoopMontage);

    GetWorld()->GetTimerManager().SetTimer(
        GroggyTimerHandle, this, &URVHitReactionComponent::EndGroggy, GroggyDuration, false);
}

void URVHitReactionComponent::OnGroggyEndMontageBlendingOut(UAnimMontage*, bool)
{
    OnGroggySequenceCompleted.Broadcast();
}