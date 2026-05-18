#include "Character/Enemy/RVBossCharacter.h"
#include "AI/RVAIController.h"
#include "Animation/AnimInstance.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVHitReactionComponent.h"
#include "Data/RVBossDataAsset.h"
#include "Data/RVHitReactionAnimDataAsset.h"
#include "Data/RVEnemyStatRow.h"
#include "GameFramework/CharacterMovementComponent.h"

ARVBossCharacter::ARVBossCharacter()
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = ARVAIController::StaticClass();
}

void ARVBossCharacter::BeginPlay()
{
	Super::BeginPlay();

	ensureMsgf(IsValid(BossData),
	           TEXT("[%s] BossData not assigned"), *GetNameSafe(this));
	ensureMsgf(IsValid(BossData->HitReactionAnimData),
	           TEXT("[%s] BossData.HitReactionAnimData not assigned"), *GetNameSafe(this));
	ensureMsgf(!BossData->EnemyStatRowHandle.IsNull(),
	           TEXT("[%s] BossData.EnemyStatRowHandle not assigned"), *GetNameSafe(this));

	const FRVEnemyStatRow* EnemyStat = BossData->GetEnemyStatRow();
	if (ensureMsgf(EnemyStat, TEXT("[%s] EnemyStatRow resolve failed — check DT_EnemyStats row name"), *GetNameSafe(this)))
	{
		AttributeComponent->InitFromValues(EnemyStat->MaxHP, EnemyStat->MaxPoise);
		GetCharacterMovement()->MaxWalkSpeed = EnemyStat->MoveSpeed;
		CombatStateComponent->SetCombatStat(
			EnemyStat->BaseDamage,
			EnemyStat->BasePoiseDamage,
			EnemyStat->AttackRadius);
		HitReactionComponent->SetStaggerDuration(EnemyStat->StaggerDuration);
	}

	AttributeComponent->OnHealthChanged.AddDynamic(this, &ARVBossCharacter::CheckPhaseTransition);
	AttributeComponent->OnPoiseDepleted.AddDynamic(this, &ARVBossCharacter::OnPoiseDepleted);
}

URVHitReactionAnimDataAsset* ARVBossCharacter::GetHitReactionAnimData() const
{
	return IsValid(BossData) ? BossData->HitReactionAnimData : nullptr;
}

//--- Phase Attack ------------------------------------------------------------

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

	if (!PhaseAttacks || PhaseAttacks->Patterns.IsEmpty()) { return; }

	const int32 PatternIndex = FMath::RandRange(0, PhaseAttacks->Patterns.Num() - 1);
	const FRVBossAttackPattern& Pattern = PhaseAttacks->Patterns[PatternIndex];

	if (Pattern.ComboMontages.IsEmpty()) { return; }

	ActiveComboMontages = Pattern.ComboMontages;
	ActiveComboIndex = 0;

	PlayComboMontageAt(0);
}

void ARVBossCharacter::PlayComboMontageAt(int32 InIndex)
{
	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInst)) { return; }

	UAnimMontage* Montage = ActiveComboMontages[InIndex];
	if (!IsValid(Montage)) { return; }

	CombatStateComponent->AddState(ERVCombatState::Attacking);
	AnimInst->Montage_Play(Montage);

	FOnMontageBlendingOutStarted BlendOutDelegate;
	BlendOutDelegate.BindUObject(this, &ARVBossCharacter::OnAttackMontageBlendingOut);
	AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, Montage);
}

void ARVBossCharacter::OnAttackMontageBlendingOut(UAnimMontage* /*InMontage*/, bool bInterrupted)
{
	CombatStateComponent->RemoveState(ERVCombatState::Attacking);

	// Chain continues only if uninterrupted and more hits remain in the pattern.
	if (!bInterrupted && ActiveComboIndex + 1 < ActiveComboMontages.Num())
	{
		++ActiveComboIndex;
		PlayComboMontageAt(ActiveComboIndex);
	}
	else
	{
		ActiveComboMontages.Empty();
		ActiveComboIndex = 0;
	}
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

	// Interrupt any active combo chain.
	ActiveComboMontages.Empty();
	ActiveComboIndex = 0;

	CombatStateComponent->AddState(ERVCombatState::Groggy);
	OnBossGroggyStarted.Broadcast();

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (IsValid(AnimInst) && IsValid(BossData->HitReactionAnimData->GroggyStartMontage))
	{
		AnimInst->Montage_Play(BossData->HitReactionAnimData->GroggyStartMontage);
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
	if (IsValid(AnimInst) && IsValid(BossData->HitReactionAnimData->GroggyEndMontage))
	{
		AnimInst->Montage_Play(BossData->HitReactionAnimData->GroggyEndMontage);
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