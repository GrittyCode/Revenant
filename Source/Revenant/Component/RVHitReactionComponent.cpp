#include "Component/RVHitReactionComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Data/RVCharacterDataAsset.h"
#include "Data/RVWeaponDataAsset.h"
#include "Data/RVWeaponAnimationDataAsset.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
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
    if (CombatStateComponent->IsInState(ERVCombatState::Knockdown | ERVCombatState::HitReaction))
    {
        return;
    }

    const bool bPoiseDepleted = AttributeComponent->ApplyPoiseDamage(InHitInfo.PoiseDamage);
    if (!bPoiseDepleted) { return; }

    CombatStateComponent->ForceEndAllActions();
    AttributeComponent->ResetPoise();

    const bool bShouldKnockdown = InHitInfo.HitType != ERVHitType::Normal
                                || !CombatStateComponent->IsGrounded()
                                || CombatStateComponent->IsInState(ERVCombatState::HitReaction);

    if (bShouldKnockdown)
    {
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

//--- Reaction Triggers -------------------------------------------------------

void URVHitReactionComponent::TriggerStagger(const FVector& InHitDirection)
{
    StaggerDirection = UKismetAnimationLibrary::CalculateDirection(
        InHitDirection, OwnerCharacter->GetActorRotation());

    CombatStateComponent->AddState(ERVCombatState::HitReaction);

    const float Duration = IsValid(CharacterData) ? CharacterData->StaggerDuration : 0.5f;
    GetWorld()->GetTimerManager().SetTimer(
        StaggerHandle,
        this,
        &URVHitReactionComponent::OnStaggerEnd,
        Duration,
        false
    );
}

void URVHitReactionComponent::TriggerKnockdown(const FVector& InHitDirection)
{
    const URVWeaponDataAsset* WeaponData = EquipmentComponent->GetCurrentWeaponData();
    if (!IsValid(WeaponData) || !IsValid(WeaponData->AnimationDataAsset)) { return; }

    URVWeaponAnimationDataAsset* AnimData = WeaponData->AnimationDataAsset;
    if (!IsValid(AnimData->KnockdownMontage)) { return; }

    UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }
	
	OwnerCharacter->SetActorRotation(FRotationMatrix::MakeFromX(InHitDirection).Rotator());
	CombatStateComponent->AddState(ERVCombatState::Knockdown);
	
    AnimInst->Montage_Play(AnimData->KnockdownMontage);

    FOnMontageBlendingOutStarted BlendingOutDelegate;
    BlendingOutDelegate.BindUObject(this, &URVHitReactionComponent::OnKnockdownMontageBlendingOut);
    AnimInst->Montage_SetBlendingOutDelegate(BlendingOutDelegate, AnimData->KnockdownMontage);
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
}