#include "Component/RVHitreactioncomponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVAttributeComponent.h"
#include "Animation/RVAnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Data/RVCharacterDataAsset.h"
#include "GameFramework/Character.h"

URVHitReactionComponent::URVHitReactionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URVHitReactionComponent::BeginPlay()
{
    Super::BeginPlay();
    // References injected via InitReferences() from ARVCharacterBase::BeginPlay.
}

void URVHitReactionComponent::InitReferences(
    ACharacter* InOwnerCharacter,
    URVCombatStateComponent* InCombatStateComponent,
    URVAttributeComponent* InAttributeComponent,
    URVCharacterDataAsset* InCharacterData)
{
    OwnerCharacter        = InOwnerCharacter;
    CombatStateComponent  = InCombatStateComponent;
    AttributeComponent    = InAttributeComponent;
    CharacterData         = InCharacterData;
}

//--- Main Entry Point --------------------------------------------------------

void URVHitReactionComponent::HandleHit(const FRVHitInfo& InHitInfo)
{
    // 1. Physical Reaction — fires on every hit regardless of combat state.
    //    Drives the ABP additive layer to create a micro-flinch without
    //    interrupting the current action.
    TriggerPhysicalReaction(InHitInfo.HitDirection);

    // 2. Do not override an active Groggy or Knockdown state with a new reaction.
    //    The character is already in a severe hit state; stacking another would
    //    cancel the get-up sequence or extend Groggy unpredictably.
    if (CombatStateComponent->IsInState(
        ERVCombatState::Groggy | ERVCombatState::Knockdown | ERVCombatState::HitReaction))
    {
        return;
    }

    // 3. Apply poise damage. Returns true only if poise reached 0.
    const bool bPoiseDepleted = AttributeComponent->ApplyPoiseDamage(InHitInfo.PoiseDamage);
    if (!bPoiseDepleted) { return; }

    // 4. Poise depleted — interrupt all ongoing actions before playing reaction montage.
    //    OnForceEnd subscribers (Combo, HeavyAttack, Dodge, Guard, Sprint) self-clean.
    CombatStateComponent->ForceEndAllActions();

    // 5. Select reaction type.
    //    Knockdown: forced by weapon flag (e.g. heavy auto-release) OR airborne at time of hit.
    //    Groggy:    poise depleted for the Nth time without Knockdown (N = GroggyThreshold).
    //    Stagger:   all other poise depletions.
    const bool bShouldKnockdown = InHitInfo.bForceKnockdown
                                || !CombatStateComponent->IsGrounded();

    if (bShouldKnockdown)
    {
        AttributeComponent->ResetPoise();
        TriggerKnockdown();
    }
    else
    {
        ++StaggerCount;

        const int32 Threshold = IsValid(CharacterData) ? CharacterData->GroggyThreshold : 2;
        if (StaggerCount >= Threshold)
        {
            StaggerCount = 0;
            AttributeComponent->ResetPoise();
            TriggerGroggy();
        }
        else
        {
            AttributeComponent->ResetPoise();
            TriggerStagger(InHitInfo.HitDirection);
        }
    }
}

//--- Reaction Triggers -------------------------------------------------------

void URVHitReactionComponent::TriggerPhysicalReaction(const FVector& InHitDirection)
{
    // Pass hit direction to URVAnimInstance so the ABP additive flinch layer
    // can blend the correct directional pose without interrupting the action.
    URVAnimInstance* AnimInst = Cast<URVAnimInstance>(
        OwnerCharacter->GetMesh()->GetAnimInstance());
    if (!IsValid(AnimInst)) { return; }

    AnimInst->TriggerHitReaction(InHitDirection);
}

void URVHitReactionComponent::TriggerStagger(const FVector& InHitDirection)
{
    UAnimMontage* Montage = SelectStaggerMontage(InHitDirection);
    if (!IsValid(Montage)) { return; }

    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    CombatStateComponent->AddState(ERVCombatState::HitReaction);
    AttributeComponent->PauseStaminaRegen();

    AnimInst->Montage_Play(Montage);

    // Bind cleanup to montage blend-out so state is cleared as the montage exits,
    // giving the transition into the next idle/locomotion pose time to interpolate.
    FOnMontageBlendingOutStarted BlendingOutDelegate;
    BlendingOutDelegate.BindUObject(this, &URVHitReactionComponent::OnStaggerMontageBlendingOut);
    AnimInst->Montage_SetBlendingOutDelegate(BlendingOutDelegate, Montage);
}

void URVHitReactionComponent::TriggerGroggy()
{
    if (!IsValid(CharacterData) || !IsValid(CharacterData->GroggyMontage)) { return; }

    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    CombatStateComponent->AddState(ERVCombatState::Groggy);
    AttributeComponent->PauseStaminaRegen();

    AnimInst->Montage_Play(CharacterData->GroggyMontage);

    // Timer-based recovery: Groggy is held for GroggyDuration, not montage-driven,
    // so that execution windows are predictable regardless of montage length.
    GetWorld()->GetTimerManager().SetTimer(
        GroggyHandle,
        this,
        &URVHitReactionComponent::EndGroggy,
        CharacterData->GroggyDuration,
        false
    );
}

void URVHitReactionComponent::TriggerKnockdown()
{
    if (!IsValid(CharacterData) || !IsValid(CharacterData->KnockdownMontage)) { return; }

    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    CombatStateComponent->AddState(ERVCombatState::Knockdown);
    AttributeComponent->PauseStaminaRegen();

    AnimInst->Montage_Play(CharacterData->KnockdownMontage);

    FOnMontageBlendingOutStarted BlendingOutDelegate;
    BlendingOutDelegate.BindUObject(this, &URVHitReactionComponent::OnKnockdownMontageBlendingOut);
    AnimInst->Montage_SetBlendingOutDelegate(BlendingOutDelegate, CharacterData->KnockdownMontage);
}

//--- Montage End Callbacks ---------------------------------------------------

void URVHitReactionComponent::OnStaggerMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
    CombatStateComponent->RemoveState(ERVCombatState::HitReaction);
    AttributeComponent->ResumeStaminaRegen();
}

void URVHitReactionComponent::EndGroggy()
{
    // Stop the looping Groggy montage before restoring state.
    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (IsValid(AnimInst) && IsValid(CharacterData) && IsValid(CharacterData->GroggyMontage))
    {
        AnimInst->Montage_Stop(0.3f, CharacterData->GroggyMontage);
    }

    CombatStateComponent->RemoveState(ERVCombatState::Groggy);
    AttributeComponent->ResumeStaminaRegen();
}

void URVHitReactionComponent::OnKnockdownMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
    // Sequentially play the get-up montage after the fall ends.
    // Knockdown state is only cleared after get-up completes, not here.
    if (!IsValid(CharacterData) || !IsValid(CharacterData->GetUpMontage)) { return; }

    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    AnimInst->Montage_Play(CharacterData->GetUpMontage);

    FOnMontageBlendingOutStarted GetUpDelegate;
    GetUpDelegate.BindUObject(this, &URVHitReactionComponent::OnGetUpMontageBlendingOut);
    AnimInst->Montage_SetBlendingOutDelegate(GetUpDelegate, CharacterData->GetUpMontage);
}

void URVHitReactionComponent::OnGetUpMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
    CombatStateComponent->RemoveState(ERVCombatState::Knockdown);
    AttributeComponent->ResumeStaminaRegen();
}

//--- Helpers -----------------------------------------------------------------

UAnimMontage* URVHitReactionComponent::SelectStaggerMontage(const FVector& InHitDirection) const
{
    if (!IsValid(CharacterData)) { return nullptr; }

    // InHitDirection is world-space FROM instigator TOWARD target.
    // We want the direction the hit came from relative to this character's axes,
    // so we negate it to get the vector FROM this character TOWARD the attacker.
    const FVector HitOriginDir = -InHitDirection;

    const float ForwardDot = FVector::DotProduct(OwnerCharacter->GetActorForwardVector(), HitOriginDir);
    const float RightDot   = FVector::DotProduct(OwnerCharacter->GetActorRightVector(),   HitOriginDir);

    UAnimMontage* Selected = nullptr;

    if (FMath::Abs(ForwardDot) >= FMath::Abs(RightDot))
    {
        // Hit from front or back axis
        Selected = (ForwardDot >= 0.f) ? CharacterData->StaggerMontage_F
                                       : CharacterData->StaggerMontage_B;
    }
    else
    {
        // Hit from left or right axis
        Selected = (RightDot >= 0.f) ? CharacterData->StaggerMontage_R
                                     : CharacterData->StaggerMontage_L;
    }

    // Fallback: if the directional slot is unassigned, use the front stagger
    // so the character always reacts rather than playing nothing.
    return IsValid(Selected) ? Selected : CharacterData->StaggerMontage_F.Get();
}