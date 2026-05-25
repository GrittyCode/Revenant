#include "Component/RVHitReactionComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVAttributeComponent.h"
#include "Data/RVHitReactionAnimDataAsset.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "KismetAnimationLibrary.h"

URVHitReactionComponent::URVHitReactionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URVHitReactionComponent::BeginPlay()
{
    Super::BeginPlay();
}

void URVHitReactionComponent::InitReferences(
    ACharacter* InOwnerCharacter,
    URVCombatStateComponent* InCombatStateComponent,
    URVAttributeComponent* InAttributeComponent,
    URVHitReactionAnimDataAsset* InHitReactionAnimData,
    float InStaggerDuration,
    float InStaggerThreshold,
    float InKnockdownThreshold)
{
    OwnerCharacter       = InOwnerCharacter;
    CombatStateComponent = InCombatStateComponent;
    AttributeComponent   = InAttributeComponent;
    HitReactionAnimData  = InHitReactionAnimData;
    StaggerDuration      = InStaggerDuration;
    StaggerThreshold     = InStaggerThreshold;
    KnockdownThreshold   = InKnockdownThreshold;
}

//--- Main Entry Point --------------------------------------------------------

void URVHitReactionComponent::HandleHit(const FRVHitInfo& InHitInfo)
{
    if (CombatStateComponent->HasState(ERVCombatState::Knockdown)) { return; }

    const float MaxPoise = AttributeComponent->GetMaxPoise();

    // Single-hit knockdown: one attack delivers enough poise damage relative to MaxPoise.
    const bool bSingleHitKnockdown = MaxPoise > 0.f
        && (InHitInfo.PoiseDamage / MaxPoise) >= KnockdownThreshold;

    // Apply poise damage. Also fires OnPoiseDepleted for boss groggy if poise hits 0.
    AttributeComponent->ApplyPoiseDamage(InHitInfo.PoiseDamage);

    const float PoiseRatio = AttributeComponent->GetPoiseRatio();

    // Escalate to knockdown if already staggering or airborne.
    const bool bShouldKnockdown = bSingleHitKnockdown
        || !CombatStateComponent->IsGrounded()
        || CombatStateComponent->HasState(ERVCombatState::HitReaction);

    const bool bReactionNeeded = bShouldKnockdown || (PoiseRatio <= StaggerThreshold);
    if (!bReactionNeeded) { return; }

    const ERVHitReactCapability Required = bShouldKnockdown
        ? ERVHitReactCapability::Knockdown
        : ERVHitReactCapability::Stagger;

    // Poise damage accumulates even when reactions are disabled (e.g. boss Groggy capability only).
    if (!CanHitReact(Required)) { return; }

    CombatStateComponent->ForceEndAllActions();
    AttributeComponent->ResetPoise();

    if (bShouldKnockdown)
    {
        if (CombatStateComponent->HasState(ERVCombatState::HitReaction))
        {
            GetWorld()->GetTimerManager().ClearTimer(StaggerHandle);
            CombatStateComponent->RemoveState(ERVCombatState::HitReaction);
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

    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    CombatStateComponent->AddState(ERVCombatState::HitReaction);

    AnimInst->Montage_Play(InMontage);

    FOnMontageBlendingOutStarted BlendingOutDelegate;
    BlendingOutDelegate.BindUObject(this, &URVHitReactionComponent::OnStaggerMontageBlendingOut);
    AnimInst->Montage_SetBlendingOutDelegate(BlendingOutDelegate, InMontage);
}

//--- Groggy ------------------------------------------------------------------

void URVHitReactionComponent::TriggerGroggy(float InGroggyDuration)
{
    if (!IsValid(HitReactionAnimData)) { return; }

    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
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

    if (!IsValid(HitReactionAnimData)) { return; }

    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    UAnimMontage* LoopMontage = HitReactionAnimData->GroggyStunLoopMontage;
    if (IsValid(LoopMontage)) { AnimInst->Montage_Stop(0.2f, LoopMontage); }

    UAnimMontage* EndMontage = HitReactionAnimData->GroggyStunEndMontage;
    if (!IsValid(EndMontage))
    {
        OnGroggySequenceCompleted.Broadcast();
        return;
    }

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
        InHitDirection, OwnerCharacter->GetActorRotation());

    CombatStateComponent->AddState(ERVCombatState::HitReaction);

    GetWorld()->GetTimerManager().SetTimer(
        StaggerHandle,
        this,
        &URVHitReactionComponent::OnStaggerEnd,
        StaggerDuration,
        false);
}

void URVHitReactionComponent::TriggerKnockdown(const FVector& InHitDirection)
{
    if (!IsValid(HitReactionAnimData)) { return; }

    UAnimMontage* KnockdownMontage = HitReactionAnimData->KnockdownMontage;
    if (!IsValid(KnockdownMontage)) { return; }

    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    OwnerCharacter->SetActorRotation(FRotationMatrix::MakeFromX(InHitDirection).Rotator());
    CombatStateComponent->AddState(ERVCombatState::Knockdown);

    AnimInst->Montage_Play(KnockdownMontage);

    FOnMontageBlendingOutStarted BlendingOutDelegate;
    BlendingOutDelegate.BindUObject(this, &URVHitReactionComponent::OnKnockdownMontageBlendingOut);
    AnimInst->Montage_SetBlendingOutDelegate(BlendingOutDelegate, KnockdownMontage);
}

//--- Callbacks ---------------------------------------------------------------

void URVHitReactionComponent::OnStaggerEnd()
{
    CombatStateComponent->RemoveState(ERVCombatState::HitReaction);
}

void URVHitReactionComponent::OnStaggerMontageBlendingOut(UAnimMontage* /*Montage*/, bool /*bInterrupted*/)
{
    CombatStateComponent->RemoveState(ERVCombatState::HitReaction);
}

void URVHitReactionComponent::OnKnockdownMontageBlendingOut(UAnimMontage* /*Montage*/, bool /*bInterrupted*/)
{
    if (!IsValid(HitReactionAnimData)) { return; }

    UAnimMontage* GetUpMontage = HitReactionAnimData->GetUpMontage;
    if (!IsValid(GetUpMontage)) { return; }

    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    AnimInst->Montage_Play(GetUpMontage);

    FOnMontageBlendingOutStarted GetUpDelegate;
    GetUpDelegate.BindUObject(this, &URVHitReactionComponent::OnGetUpMontageBlendingOut);
    AnimInst->Montage_SetBlendingOutDelegate(GetUpDelegate, GetUpMontage);
}

void URVHitReactionComponent::OnGetUpMontageBlendingOut(UAnimMontage* /*Montage*/, bool /*bInterrupted*/)
{
    CombatStateComponent->RemoveState(ERVCombatState::Knockdown);
}

void URVHitReactionComponent::OnGroggyStartMontageBlendingOut(UAnimMontage* /*Montage*/, bool bInterrupted)
{
    if (bInterrupted) { return; }

    if (!IsValid(HitReactionAnimData)) { return; }

    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    UAnimMontage* LoopMontage = HitReactionAnimData->GroggyStunLoopMontage;
    if (!IsValid(LoopMontage)) { return; }

    AnimInst->Montage_Play(LoopMontage);

    GetWorld()->GetTimerManager().SetTimer(
        GroggyTimerHandle, this, &URVHitReactionComponent::EndGroggy, GroggyDuration, false);
}

void URVHitReactionComponent::OnGroggyEndMontageBlendingOut(UAnimMontage* /*Montage*/, bool /*bInterrupted*/)
{
    OnGroggySequenceCompleted.Broadcast();
}