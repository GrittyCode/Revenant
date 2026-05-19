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
	if (ensureMsgf(EnemyStat,
		TEXT("[%s] EnemyStatRow resolve failed — check DT_EnemyStats row name"), *GetNameSafe(this)))
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

	if (!bInterrupted && ActiveComboIndex + 1 < ActiveComboMontages.Num())
	{
		++ActiveComboIndex;
		PlayComboMontageAt(ActiveComboIndex);
	}
	else
	{
		ActiveComboMontages.Empty();
		ActiveComboIndex  = 0;
		LastAttackEndTime = GetWorld()->GetTimeSeconds();
	}
}

//--- ForceEnd ----------------------------------------------------------------

void ARVSevarogCharacter::ForceEndCurrentAction()
{
	ActiveComboMontages.Empty();
	ActiveComboIndex = 0;

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (IsValid(AnimInst))
	{
		AnimInst->Montage_Stop(0.2f);
	}

	CombatStateComponent->RemoveState(ERVCombatState::Attacking);
	LastAttackEndTime = GetWorld()->GetTimeSeconds();
}

//--- Phase Attack ------------------------------------------------------------

void ARVSevarogCharacter::ExecutePhaseAttack()
{
	if (bIsGroggy)     { return; }
	if (IsAttacking()) { return; }

	const bool bWasJustRushed = bJustRushed;
	bJustRushed = false;

	if (bWasJustRushed && !SevarogData->RushFollowupAttacks.Patterns.IsEmpty())
	{
		const int32 Index = FMath::RandRange(0, SevarogData->RushFollowupAttacks.Patterns.Num() - 1);
		StartComboChain(SevarogData->RushFollowupAttacks.Patterns[Index].ComboMontages);
		return;
	}

	const FRVBossPhaseAttacks* PhaseAttacks = nullptr;
	switch (CurrentPhase)
	{
	case ERVBossPhase::Phase1: PhaseAttacks = &SevarogData->Phase1Attacks; break;
	case ERVBossPhase::Phase2: PhaseAttacks = &SevarogData->Phase2Attacks; break;
	case ERVBossPhase::Phase3: PhaseAttacks = &SevarogData->Phase3Attacks; break;
	}

	if (!PhaseAttacks || PhaseAttacks->Patterns.IsEmpty()) { return; }

	const int32 Index = FMath::RandRange(0, PhaseAttacks->Patterns.Num() - 1);
	StartComboChain(PhaseAttacks->Patterns[Index].ComboMontages);
}

void ARVSevarogCharacter::ExecuteRushAttack()
{
	if (bIsGroggy)     { return; }
	if (IsAttacking()) { return; }
	if (!IsValid(SevarogData->RushAttackMontage)) { return; }

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
	bIsGroggy   = true;
	bJustRushed = false;
	CurrentPoiseDepletionCount = 0;

	if (IsAttacking())
	{
		ForceEndCurrentAction();
	}

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
		false);
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
	bIsRushing  = false;
	bJustRushed = true;
	GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;
	LastRushEndTime = GetWorld()->GetTimeSeconds();
}

//--- Cooldown queries --------------------------------------------------------

float ARVSevarogCharacter::GetAttackCooldown() const
{
	if (!IsValid(SevarogData)) { return 0.f; }

	switch (CurrentPhase)
	{
	case ERVBossPhase::Phase1: return SevarogData->Phase1AttackCooldown;
	case ERVBossPhase::Phase2: return SevarogData->Phase2AttackCooldown;
	case ERVBossPhase::Phase3: return SevarogData->Phase3AttackCooldown;
	}
	return SevarogData->Phase1AttackCooldown;
}

bool ARVSevarogCharacter::IsAttackOnCooldown() const
{
	if (!IsValid(SevarogData)) { return false; }
	return GetWorld()->GetTimeSeconds() - LastAttackEndTime < GetAttackCooldown();
}

bool ARVSevarogCharacter::IsRushOnCooldown() const
{
	if (!IsValid(SevarogData)) { return false; }
	return GetWorld()->GetTimeSeconds() - LastRushEndTime < SevarogData->RushCooldown;
}

bool ARVSevarogCharacter::IsSoulSiphonOnCooldown() const
{
	if (!IsValid(SevarogData)) { return false; }
	return GetWorld()->GetTimeSeconds() - LastSoulSiphonTime < SevarogData->SoulSiphonCooldown;
}

bool ARVSevarogCharacter::IsSubjugationOnCooldown() const
{
	if (!IsValid(SevarogData)) { return false; }
	return GetWorld()->GetTimeSeconds() - LastSubjugationTime < SevarogData->SubjugationCooldown;
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

	FOnMontageBlendingOutStarted BlendOutDelegate;
	BlendOutDelegate.BindUObject(this, &ARVSevarogCharacter::OnSoulSiphonMontageBlendingOut);
	AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, SevarogData->SoulSiphonMontage);
}

void ARVSevarogCharacter::OnSoulSiphonMontageBlendingOut(UAnimMontage* /*InMontage*/, bool bInterrupted)
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
	BlendOutDelegate.BindUObject(this, &ARVSevarogCharacter::OnSubjugationMontageBlendingOut);
	AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, SevarogData->SubjugationMontage);
}

void ARVSevarogCharacter::OnSubjugationMontageBlendingOut(UAnimMontage* /*InMontage*/, bool /*bInterrupted*/)
{
	CombatStateComponent->RemoveState(ERVCombatState::Attacking);
	LastAttackEndTime   = GetWorld()->GetTimeSeconds();
	LastSubjugationTime = GetWorld()->GetTimeSeconds();
}

void ARVSevarogCharacter::SpawnGroundField()
{
	// SpawnActor<ARVGroundField> at boss forward + offset using SevarogData->GroundField* params.
}