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
            if (UWorld* World = GetWorld()) { World->GetTimerManager().ClearTimer(StaggerHandle); }
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
    if (!ensureMsgf(IsValid(InMontage),
        TEXT("[%s] TriggerStaggerWithMontage: InMontage is null — check GuardBreakMontage assignment"),
        *GetNameSafe(OwnerBase))) { return; }

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
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                GroggyTimerHandle, this, &URVHitReactionComponent::EndGroggy, GroggyDuration, false);
        }
        return;
    }

    AnimInst->Montage_Play(StartMontage);

    FOnMontageBlendingOutStarted BlendOutDelegate;
    BlendOutDelegate.BindUObject(this, &URVHitReactionComponent::OnGroggyStartMontageBlendingOut);
    AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, StartMontage);
}

void URVHitReactionComponent::EndGroggy()
{
    if (UWorld* World = GetWorld()) { World->GetTimerManager().ClearTimer(GroggyTimerHandle); }

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
    if (UWorld* World = GetWorld()) { World->GetTimerManager().ClearTimer(GroggyTimerHandle); }
}

//--- Reaction Triggers -------------------------------------------------------

void URVHitReactionComponent::TriggerStagger(const FVector& InHitDirection)
{
    StaggerDirection = UKismetAnimationLibrary::CalculateDirection(
        InHitDirection, OwnerBase->GetActorRotation());

    OwnerBase->AddCombatState(ERVCombatState::HitReaction);

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            StaggerHandle, this, &URVHitReactionComponent::OnStaggerEnd, StaggerDuration, false);
    }
}

void URVHitReactionComponent::TriggerKnockdown(const FVector& InHitDirection)
{
    if (!ensureMsgf(IsValid(HitReactionAnimData),
        TEXT("[%s] TriggerKnockdown: HitReactionAnimData not assigned"),
        *GetNameSafe(OwnerBase))) { return; }

    UAnimMontage* KnockdownMontage = HitReactionAnimData->KnockdownMontage;
    if (!ensureMsgf(IsValid(KnockdownMontage),
        TEXT("[%s] TriggerKnockdown: KnockdownMontage not assigned"),
        *GetNameSafe(OwnerBase))) { return; }

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
    if (!ensureMsgf(IsValid(HitReactionAnimData),
        TEXT("[%s] OnKnockdownMontageBlendingOut: HitReactionAnimData not assigned"),
        *GetNameSafe(OwnerBase))) { return; }

    UAnimMontage* GetUpMontage = HitReactionAnimData->GetUpMontage;
    if (!ensureMsgf(IsValid(GetUpMontage),
        TEXT("[%s] OnKnockdownMontageBlendingOut: GetUpMontage not assigned"),
        *GetNameSafe(OwnerBase))) { return; }

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

    if (!ensureMsgf(IsValid(HitReactionAnimData),
        TEXT("[%s] OnGroggyStartMontageBlendingOut: HitReactionAnimData not assigned"),
        *GetNameSafe(OwnerBase))) { return; }

    UAnimMontage* LoopMontage = HitReactionAnimData->GroggyStunLoopMontage;
    if (!ensureMsgf(IsValid(LoopMontage),
        TEXT("[%s] OnGroggyStartMontageBlendingOut: GroggyStunLoopMontage not assigned"),
        *GetNameSafe(OwnerBase))) { return; }

    UAnimInstance* AnimInst = GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    AnimInst->Montage_Play(LoopMontage);

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            GroggyTimerHandle, this, &URVHitReactionComponent::EndGroggy, GroggyDuration, false);
    }
}

void URVHitReactionComponent::OnGroggyEndMontageBlendingOut(UAnimMontage*, bool)
{
    OnGroggySequenceCompleted.Broadcast();
}