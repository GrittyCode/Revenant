#include "Character/Enemy/RVBossCharacter.h"

#include "AI/RVAIController.h"
#include "Animation/AnimInstance.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Data/RVBossDataAsset.h"
#include "Data/RVCombatDataAsset.h"

ARVBossCharacter::ARVBossCharacter()
{
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = ARVAIController::StaticClass();
}

void ARVBossCharacter::BeginPlay()
{
    // Super::BeginPlay calls GetCombatData() via virtual dispatch → BossData->CombatData returned.
    // GetWeaponTraceMesh() not overridden → base returns GetMesh() (Sevarog skeleton).
    Super::BeginPlay();

    ensureMsgf(IsValid(BossData),
        TEXT("[%s] BossData not assigned"), *GetNameSafe(this));
    ensureMsgf(IsValid(BossData->CombatData),
        TEXT("[%s] BossData.CombatData not assigned"), *GetNameSafe(this));

    AttributeComponent->OnHealthChanged.AddDynamic(this, &ARVBossCharacter::CheckPhaseTransition);
    AttributeComponent->OnPoiseDepleted.AddDynamic(this, &ARVBossCharacter::OnPoiseDepleted);
}

URVCombatDataAsset* ARVBossCharacter::GetCombatData() const
{
    return IsValid(BossData) ? BossData->CombatData : nullptr;
}

//--- StateTree Task Interface ------------------------------------------------

void ARVBossCharacter::ExecuteBossAttack(UAnimMontage* InAttackMontage)
{
    if (!IsValid(InAttackMontage)) { return; }
    if (bIsGroggy) { return; }
    if (IsAttacking()) { return; }

    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst)) { return; }

    CombatStateComponent->AddState(ERVCombatState::Attacking);
    AnimInst->Montage_Play(InAttackMontage);

    FOnMontageBlendingOutStarted BlendOutDelegate;
    BlendOutDelegate.BindUObject(this, &ARVBossCharacter::OnAttackMontageBlendingOut);
    AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, InAttackMontage);
}

void ARVBossCharacter::OnAttackMontageBlendingOut(UAnimMontage* /*InMontage*/, bool /*bInterrupted*/)
{
    CombatStateComponent->RemoveState(ERVCombatState::Attacking);
}

void ARVBossCharacter::ExecutePhaseAttack()
{
	if (bIsGroggy) { return; }
	if (IsAttacking()) { return; }

	const FRVBossPhaseAttacks* PhaseAttacks = nullptr;

	switch (CurrentPhase)
	{
	case ERVBossPhase::Phase1: PhaseAttacks = &BossData->Phase1Attacks; break;
	case ERVBossPhase::Phase2: PhaseAttacks = &BossData->Phase2Attacks; break;
	case ERVBossPhase::Phase3: PhaseAttacks = &BossData->Phase3Attacks; break;
	}

	if (!PhaseAttacks || PhaseAttacks->Montages.IsEmpty()) { return; }

	const int32 Index = FMath::RandRange(0, PhaseAttacks->Montages.Num() - 1);
	ExecuteBossAttack(PhaseAttacks->Montages[Index]);
}

bool ARVBossCharacter::IsAttacking() const
{
    return CombatStateComponent->HasState(ERVCombatState::Attacking);
}

//--- Groggy ------------------------------------------------------------------

void ARVBossCharacter::StartGroggy()
{
    if (bIsGroggy) { return; }
    bIsGroggy = true;
    CurrentPoiseDepletionCount = 0;

    CombatStateComponent->AddState(ERVCombatState::Groggy);
    OnBossGroggyStarted.Broadcast();

    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    if (IsValid(AnimInst) && IsValid(BossData->GroggyStartMontage))
    {
        AnimInst->Montage_Play(BossData->GroggyStartMontage);
    }

    GetWorldTimerManager().SetTimer(
        GroggyTimerHandle,
        this,
        &ARVBossCharacter::EndGroggy,
        BossData->GroggyDuration,
        false
    );
}

void ARVBossCharacter::EndGroggy()
{
    if (!bIsGroggy) { return; }
    bIsGroggy = false;

    CombatStateComponent->RemoveState(ERVCombatState::Groggy);

    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    if (IsValid(AnimInst) && IsValid(BossData->GroggyEndMontage))
    {
        AnimInst->Montage_Play(BossData->GroggyEndMontage);
    }

    OnBossGroggyEnded.Broadcast();
}

//--- Phase Transition --------------------------------------------------------

void ARVBossCharacter::SetBossPhase(ERVBossPhase InNewPhase)
{
    if (CurrentPhase == InNewPhase) { return; }
    CurrentPhase = InNewPhase;
    OnBossPhaseChanged.Broadcast(CurrentPhase);
}

void ARVBossCharacter::CheckPhaseTransition(float /*InNewHealth*/, float /*InDelta*/)
{
    const float HealthRatio = AttributeComponent->GetHealthPercent();

    if (CurrentPhase == ERVBossPhase::Phase1 && HealthRatio <= BossData->Phase2Threshold)
    {
        SetBossPhase(ERVBossPhase::Phase2);
        return;
    }

    if (CurrentPhase == ERVBossPhase::Phase2 && HealthRatio <= BossData->Phase3Threshold)
    {
        SetBossPhase(ERVBossPhase::Phase3);
    }
}

void ARVBossCharacter::OnPoiseDepleted()
{
    if (bIsGroggy) { return; }

    ++CurrentPoiseDepletionCount;
    if (CurrentPoiseDepletionCount >= BossData->GroggyPoiseDepletionCount)
    {
        StartGroggy();
    }
}