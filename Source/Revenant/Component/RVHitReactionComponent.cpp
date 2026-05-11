#include "Component/RVHitReactionComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Animation/RVAnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Data/RVCharacterDataAsset.h"
#include "Data/RVWeaponDataAsset.h"
#include "Data/RVWeaponAnimationDataAsset.h"
#include "GameFramework/Character.h"

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
    URVEquipmentComponent* InEquipmentComponent,
    URVCharacterDataAsset* InCharacterData)
{
    OwnerCharacter       = InOwnerCharacter;
    CombatStateComponent = InCombatStateComponent;
    AttributeComponent   = InAttributeComponent;
    EquipmentComponent   = InEquipmentComponent;
    CharacterData        = InCharacterData;
}

//--- Main Entry Point --------------------------------------------------------

void URVHitReactionComponent::HandleHit(const FRVHitInfo& InHitInfo)
{
    // Physical reaction fires on every hit — micro-flinch in ABP without blocking actions.
    TriggerPhysicalReaction(InHitInfo.HitDirection);

    // Do not stack a new reaction on top of Groggy or Knockdown.
    // Stacking would cancel the get-up sequence or extend Groggy unpredictably.
    if (CombatStateComponent->IsInState(
        ERVCombatState::Groggy | ERVCombatState::Knockdown | ERVCombatState::HitReaction))
    {
        return;
    }

    const bool bPoiseDepleted = AttributeComponent->ApplyPoiseDamage(InHitInfo.PoiseDamage);
    if (!bPoiseDepleted) { return; }

    // Interrupt all ongoing actions before playing reaction montage.
    CombatStateComponent->ForceEndAllActions();

    // Knockdown conditions: Heavy hit type, airborne, or stagger-on-stagger.
    const bool bShouldKnockdown = InHitInfo.HitType != ERVHitType::Normal
                                || !CombatStateComponent->IsGrounded()
                                || CombatStateComponent->IsInState(ERVCombatState::HitReaction);

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

//--- Guard-Break Entry Point -------------------------------------------------

void URVHitReactionComponent::TriggerStaggerWithMontage(UAnimMontage* InMontage)
{
    // Guard break routes through here so montage length defines recovery — no separate timer.
    if (!IsValid(InMontage)) { return; }

    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    CombatStateComponent->AddState(ERVCombatState::HitReaction);
    AttributeComponent->PauseStaminaRegen();

    AnimInst->Montage_Play(InMontage);

    FOnMontageBlendingOutStarted BlendingOutDelegate;
    BlendingOutDelegate.BindUObject(this, &URVHitReactionComponent::OnStaggerMontageBlendingOut);
    AnimInst->Montage_SetBlendingOutDelegate(BlendingOutDelegate, InMontage);
}

//--- Reaction Triggers -------------------------------------------------------

void URVHitReactionComponent::TriggerPhysicalReaction(const FVector& InHitDirection)
{
    URVAnimInstance* AnimInst = Cast<URVAnimInstance>(
        OwnerCharacter->GetMesh()->GetAnimInstance());
    if (!IsValid(AnimInst)) { return; }

    AnimInst->TriggerHitReaction(InHitDirection);
}

void URVHitReactionComponent::TriggerStagger(const FVector& InHitDirection)
{
    // Direction already set on AnimInstance by TriggerPhysicalReaction.
    // ABP samples StaggerBlendSpace at HitDirectionAngle while HitReaction state is active.
    // Duration comes from CharacterData — stagger resilience is a character stat, not weapon style.
    URVAnimInstance* AnimInst = Cast<URVAnimInstance>(
        OwnerCharacter->GetMesh()->GetAnimInstance());
    if (!IsValid(AnimInst)) { return; }

    AnimInst->TriggerHitReaction(InHitDirection);

    CombatStateComponent->AddState(ERVCombatState::HitReaction);
    AttributeComponent->PauseStaminaRegen();

    const float Duration = IsValid(CharacterData) ? CharacterData->StaggerDuration : 0.5f;
    GetWorld()->GetTimerManager().SetTimer(
        StaggerHandle,
        this,
        &URVHitReactionComponent::OnStaggerEnd,
        Duration,
        false
    );
}

void URVHitReactionComponent::TriggerGroggy()
{
    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData) || !IsValid(WeaponData->AnimationDataAsset)) { return; }

    URVWeaponAnimationDataAsset* AnimData = WeaponData->AnimationDataAsset;
    if (!IsValid(AnimData->GroggyMontage)) { return; }

    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    CombatStateComponent->AddState(ERVCombatState::Groggy);
    AttributeComponent->PauseStaminaRegen();

    AnimInst->Montage_Play(AnimData->GroggyMontage);

    // Timer-driven recovery — execution window is predictable regardless of montage length.
    GetWorld()->GetTimerManager().SetTimer(
        GroggyHandle,
        this,
        &URVHitReactionComponent::EndGroggy,
        IsValid(CharacterData) ? CharacterData->GroggyDuration : 3.f,
        false
    );
}

void URVHitReactionComponent::TriggerKnockdown()
{
    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData) || !IsValid(WeaponData->AnimationDataAsset)) { return; }

    URVWeaponAnimationDataAsset* AnimData = WeaponData->AnimationDataAsset;
    if (!IsValid(AnimData->KnockdownMontage)) { return; }

    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    CombatStateComponent->AddState(ERVCombatState::Knockdown);
    AttributeComponent->PauseStaminaRegen();

    AnimInst->Montage_Play(AnimData->KnockdownMontage);

    FOnMontageBlendingOutStarted BlendingOutDelegate;
    BlendingOutDelegate.BindUObject(this, &URVHitReactionComponent::OnKnockdownMontageBlendingOut);
    AnimInst->Montage_SetBlendingOutDelegate(BlendingOutDelegate, AnimData->KnockdownMontage);
}

//--- Callbacks ---------------------------------------------------------------

void URVHitReactionComponent::OnStaggerEnd()
{
    CombatStateComponent->RemoveState(ERVCombatState::HitReaction);
    AttributeComponent->ResumeStaminaRegen();
}

void URVHitReactionComponent::OnStaggerMontageBlendingOut(UAnimMontage* /*Montage*/, bool /*bInterrupted*/)
{
    CombatStateComponent->RemoveState(ERVCombatState::HitReaction);
    AttributeComponent->ResumeStaminaRegen();
}

void URVHitReactionComponent::EndGroggy()
{
    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (IsValid(WeaponData) && IsValid(WeaponData->AnimationDataAsset))
    {
        UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
        if (IsValid(AnimInst) && IsValid(WeaponData->AnimationDataAsset->GroggyMontage))
        {
            AnimInst->Montage_Stop(0.3f, WeaponData->AnimationDataAsset->GroggyMontage);
        }
    }

    CombatStateComponent->RemoveState(ERVCombatState::Groggy);
    AttributeComponent->ResumeStaminaRegen();
}

void URVHitReactionComponent::OnKnockdownMontageBlendingOut(UAnimMontage* /*Montage*/, bool /*bInterrupted*/)
{
    // Knockdown cleared only after get-up completes, not here.
    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData) || !IsValid(WeaponData->AnimationDataAsset)) { return; }

    URVWeaponAnimationDataAsset* AnimData = WeaponData->AnimationDataAsset;
    if (!IsValid(AnimData->GetUpMontage)) { return; }

    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    AnimInst->Montage_Play(AnimData->GetUpMontage);

    FOnMontageBlendingOutStarted GetUpDelegate;
    GetUpDelegate.BindUObject(this, &URVHitReactionComponent::OnGetUpMontageBlendingOut);
    AnimInst->Montage_SetBlendingOutDelegate(GetUpDelegate, AnimData->GetUpMontage);
}

void URVHitReactionComponent::OnGetUpMontageBlendingOut(UAnimMontage* /*Montage*/, bool /*bInterrupted*/)
{
    CombatStateComponent->RemoveState(ERVCombatState::Knockdown);
    AttributeComponent->ResumeStaminaRegen();
}