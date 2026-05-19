#include "Character/Enemy/RVSevarogCharacter.h"
#include "AI/RVAIController.h"
#include "Animation/AnimInstance.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVHitReactionComponent.h"
#include "Data/RVSevarogDataAsset.h"
#include "Data/RVHitReactionAnimDataAsset.h"
#include "Data/RVEnemyStatRow.h"
#include "GameFramework/CharacterMovementComponent.h"

ARVSevarogCharacter::ARVSevarogCharacter()
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = ARVAIController::StaticClass();
}

void ARVSevarogCharacter::BeginPlay()
{
	Super::BeginPlay();

	ensureMsgf(IsValid(SevarogData),
	           TEXT("[%s] SevarogData not assigned"), *GetNameSafe(this));
	ensureMsgf(IsValid(SevarogData->HitReactionAnimData),
	           TEXT("[%s] SevarogData.HitReactionAnimData not assigned"), *GetNameSafe(this));
	ensureMsgf(!SevarogData->EnemyStatRowHandle.IsNull(),
	           TEXT("[%s] SevarogData.EnemyStatRowHandle not assigned"), *GetNameSafe(this));

	const FRVEnemyStatRow* EnemyStat = SevarogData->GetEnemyStatRow();
	if (ensureMsgf(EnemyStat, TEXT("[%s] EnemyStatRow resolve failed — check DT_EnemyStats row name"), *GetNameSafe(this)))
	{
		AttributeComponent->InitFromValues(EnemyStat->MaxHP, EnemyStat->MaxPoise);

		NormalWalkSpeed = EnemyStat->MoveSpeed;
		GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;

		CombatStateComponent->SetCombatStat(
			EnemyStat->BaseDamage,
			EnemyStat->BasePoiseDamage,
			EnemyStat->AttackRadius);
		HitReactionComponent->SetStaggerDuration(EnemyStat->StaggerDuration);
	}

	AttributeComponent->OnHealthChanged.AddDynamic(this, &ARVSevarogCharacter::CheckPhaseTransition);
	AttributeComponent->OnPoiseDepleted.AddDynamic(this, &ARVSevarogCharacter::OnPoiseDepleted);
}

URVHitReactionAnimDataAsset* ARVSevarogCharacter::GetHitReactionAnimData() const
{
	return IsValid(SevarogData) ? SevarogData->HitReactionAnimData : nullptr;
}

//--- Internal combo chain ----------------------------------------------------

void ARVSevarogCharacter::StartComboChain(const TArray<TObjectPtr<UAnimMontage>>& InMontages)
{
	if (InMontages.IsEmpty()) { return; }

	ActiveComboMontages = InMontages;
	ActiveComboIndex    = 0;
	PlayComboMontageAt(0);
}

void ARVSevarogCharacter::PlayComboMontageAt(int32 InIndex)
{
	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInst)) { return; }

	UAnimMontage* Montage = ActiveComboMontages[InIndex];
	if (!IsValid(Montage)) { return; }

	CombatStateComponent->AddState(ERVCombatState::Attacking);
	AnimInst->Montage_Play(Montage);

	FOnMontageBlendingOutStarted BlendOutDelegate;
	BlendOutDelegate.BindUObject(this, &ARVSevarogCharacter::OnAttackMontageBlendingOut);
	AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, Montage);
}

void ARVSevarogCharacter::OnAttackMontageBlendingOut(UAnimMontage* /*InMontage*/, bool bInterrupted)
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

		// Record attack end time for cooldown tracking regardless of interruption.
		LastAttackEndTime = GetWorld()->GetTimeSeconds();
	}
}

//--- Phase Attack ------------------------------------------------------------

void ARVSevarogCharacter::ExecutePhaseAttack()
{
	if (bIsGroggy)    { return; }
	if (IsAttacking()) { return; }

	const FRVBossPhaseAttacks* PhaseAttacks = nullptr;

	switch (CurrentPhase)
	{
	case ERVBossPhase::Phase1: PhaseAttacks = &SevarogData->Phase1Attacks; break;
	case ERVBossPhase::Phase2: PhaseAttacks = &SevarogData->Phase2Attacks; break;
	case ERVBossPhase::Phase3: PhaseAttacks = &SevarogData->Phase3Attacks; break;
	}

	if (!PhaseAttacks || PhaseAttacks->Patterns.IsEmpty()) { return; }

	const int32 PatternIndex = FMath::RandRange(0, PhaseAttacks->Patterns.Num() - 1);
	StartComboChain(PhaseAttacks->Patterns[PatternIndex].ComboMontages);
}

void ARVSevarogCharacter::ExecuteRushAttack()
{
	if (bIsGroggy)    { return; }
	if (IsAttacking()) { return; }
	if (!IsValid(SevarogData->RushAttackMontage)) { return; }

	// Single montage — wrap in an array so StartComboChain handles
	// state management and LastAttackEndTime update uniformly.
	StartComboChain({ SevarogData->RushAttackMontage });
}

bool ARVSevarogCharacter::IsAttacking() const
{
	return CombatStateComponent->HasState(ERVCombatState::Attacking);
}

//--- Groggy ------------------------------------------------------------------

void ARVSevarogCharacter::StartGroggy()
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
	if (IsValid(AnimInst) && IsValid(SevarogData->HitReactionAnimData->GroggyStartMontage))
	{
		AnimInst->Montage_Play(SevarogData->HitReactionAnimData->GroggyStartMontage);
	}

	GetWorldTimerManager().SetTimer(
		GroggyTimerHandle,
		this,
		&ARVSevarogCharacter::EndGroggy,
		SevarogData->GroggyDuration,
		false
	);
}

void ARVSevarogCharacter::EndGroggy()
{
	if (!bIsGroggy) { return; }
	bIsGroggy = false;

	CombatStateComponent->RemoveState(ERVCombatState::Groggy);

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (IsValid(AnimInst) && IsValid(SevarogData->HitReactionAnimData->GroggyEndMontage))
	{
		AnimInst->Montage_Play(SevarogData->HitReactionAnimData->GroggyEndMontage);
	}

	OnBossGroggyEnded.Broadcast();
}

//--- Rush --------------------------------------------------------------------

void ARVSevarogCharacter::StartRush()
{
	bIsRushing = true;
	GetCharacterMovement()->MaxWalkSpeed = SevarogData->RushSpeed;
}

void ARVSevarogCharacter::EndRush()
{
	bIsRushing = false;
	GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;
	LastRushEndTime = GetWorld()->GetTimeSeconds();
}

//--- Backpedal ---------------------------------------------------------------

void ARVSevarogCharacter::StartBackpedal()
{
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
}

void ARVSevarogCharacter::EndBackpedal()
{
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
}

//--- Cooldown queries --------------------------------------------------------

bool ARVSevarogCharacter::IsAttackOnCooldown() const
{
	return GetWorld()->GetTimeSeconds() - LastAttackEndTime < SevarogData->AttackCooldownDuration;
}

bool ARVSevarogCharacter::IsRushOnCooldown() const
{
	return GetWorld()->GetTimeSeconds() - LastRushEndTime < SevarogData->RushCooldown;
}

bool ARVSevarogCharacter::IsSoulSiphonOnCooldown() const
{
	return GetWorld()->GetTimeSeconds() - LastSoulSiphonTime < SevarogData->SoulSiphonCooldown;
}

//--- Phase Transition --------------------------------------------------------

void ARVSevarogCharacter::SetBossPhase(ERVBossPhase InNewPhase)
{
	if (CurrentPhase == InNewPhase) { return; }
	CurrentPhase = InNewPhase;
	OnBossPhaseChanged.Broadcast(CurrentPhase);
}

void ARVSevarogCharacter::CheckPhaseTransition(float /*InNewHealth*/, float /*InDelta*/)
{
	const float HealthRatio = AttributeComponent->GetHealthPercent();

	if (CurrentPhase == ERVBossPhase::Phase1 && HealthRatio <= SevarogData->Phase2Threshold)
	{
		SetBossPhase(ERVBossPhase::Phase2);
		return;
	}

	if (CurrentPhase == ERVBossPhase::Phase2 && HealthRatio <= SevarogData->Phase3Threshold)
	{
		SetBossPhase(ERVBossPhase::Phase3);
	}
}

void ARVSevarogCharacter::OnPoiseDepleted()
{
	if (bIsGroggy) { return; }

	++CurrentPoiseDepletionCount;
	if (CurrentPoiseDepletionCount >= SevarogData->GroggyPoiseDepletionCount)
	{
		StartGroggy();
	}
}

//--- Soul Siphon -------------------------------------------------------------

void ARVSevarogCharacter::ExecuteSoulSiphon()
{
	if (IsGroggy())    { return; }
	if (IsAttacking()) { return; }
	if (!IsValid(SevarogData->SoulSiphonMontage)) { return; }

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInst)) { return; }

	CombatStateComponent->AddState(ERVCombatState::Attacking);
	AnimInst->Montage_Play(SevarogData->SoulSiphonMontage);

	// Heal only if the montage completes without interruption.
	// Record LastSoulSiphonTime regardless — prevents retry spam on interrupt.
	FOnMontageBlendingOutStarted BlendOutDelegate;
	BlendOutDelegate.BindLambda([this](UAnimMontage* /*Montage*/, bool bInterrupted)
	{
		CombatStateComponent->RemoveState(ERVCombatState::Attacking);
		LastAttackEndTime  = GetWorld()->GetTimeSeconds();
		LastSoulSiphonTime = GetWorld()->GetTimeSeconds();

		const bool bHealed = !bInterrupted;
		if (bHealed)
		{
			AttributeComponent->ApplyHealing(SevarogData->SoulSiphonHealAmount);
		}
		OnSoulSiphonCompleted.Broadcast(bHealed);
	});
	AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, SevarogData->SoulSiphonMontage);
}

//--- Subjugation -------------------------------------------------------------

void ARVSevarogCharacter::ExecuteSubjugation()
{
	if (IsGroggy())    { return; }
	if (IsAttacking()) { return; }
	if (!IsValid(SevarogData->SubjugationMontage)) { return; }

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInst)) { return; }

	CombatStateComponent->AddState(ERVCombatState::Attacking);
	AnimInst->Montage_Play(SevarogData->SubjugationMontage);

	FOnMontageBlendingOutStarted BlendOutDelegate;
	BlendOutDelegate.BindLambda([this](UAnimMontage* /*Montage*/, bool /*bInterrupted*/)
	{
		CombatStateComponent->RemoveState(ERVCombatState::Attacking);
		LastAttackEndTime = GetWorld()->GetTimeSeconds();
	});
	AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, SevarogData->SubjugationMontage);
}

void ARVSevarogCharacter::SpawnGroundField()
{
	// SpawnActor<ARVGroundField> at boss forward + offset using SevarogData->GroundField* params.
}