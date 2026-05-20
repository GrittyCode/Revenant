#include "Character/Enemy/RVSevarogCharacter.h"
#include "AI/RVAIController.h"
#include "Animation/AnimInstance.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVHitReactionComponent.h"
#include "Data/RVSevarogDataAsset.h"
#include "Data/RVHitReactionAnimDataAsset.h"
#include "Data/RVEnemyStatRow.h"
#include "Interface/RVDamageable.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

ARVSevarogCharacter::ARVSevarogCharacter()
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = ARVAIController::StaticClass();
}

void ARVSevarogCharacter::RotateToFacePlayer()
{
	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!IsValid(Player)) { return; }

	const FVector ToPlayer = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	if (ToPlayer.IsNearlyZero()) { return; }

	const FRotator CurrentRotation = GetActorRotation();
	const FRotator TargetRotation  = ToPlayer.Rotation();

	// Clamp to max delta — prevents 180° spin between combo hits
	static constexpr float MaxTurnDegrees = 60.f;
	const float DeltaYaw   = FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, TargetRotation.Yaw);
	const float ClampedYaw = FMath::Clamp(DeltaYaw, -MaxTurnDegrees, MaxTurnDegrees);

	SetActorRotation(FRotator(0.f, CurrentRotation.Yaw + ClampedYaw, 0.f));
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

//--- Weighted random selection -----------------------------------------------

int32 ARVSevarogCharacter::SelectWeightedPattern(const TArray<FRVBossAttackPattern>& InPatterns) const
{
	int32 TotalWeight = 0;
	for (const FRVBossAttackPattern& Pattern : InPatterns)
	{
		TotalWeight += FMath::Max(1, Pattern.Weight);
	}

	int32 Roll = FMath::RandRange(0, TotalWeight - 1);
	for (int32 i = 0; i < InPatterns.Num(); ++i)
	{
		Roll -= FMath::Max(1, InPatterns[i].Weight);
		if (Roll < 0)
		{
			return i;
		}
	}

	return InPatterns.Num() - 1;
}

//--- Internal combo chain ----------------------------------------------------

void ARVSevarogCharacter::StartComboChain(const TArray<TObjectPtr<UAnimMontage>>& InMontages)
{
	if (InMontages.IsEmpty()) { return; }

	ActiveComboMontages = InMontages;
	ActiveComboIndex    = 0;
	bIsComboChaining    = false;
	PlayComboMontageAt(0);
}

void ARVSevarogCharacter::PlayComboMontageAt(int32 InIndex)
{
	RotateToFacePlayer();

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

void ARVSevarogCharacter::TryChainCombo()
{
	if (ActiveComboIndex + 1 >= ActiveComboMontages.Num()) { return; }

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInst)) { return; }
	
	bIsComboChaining = true;

	++ActiveComboIndex;
	PlayComboMontageAt(ActiveComboIndex);
}

void ARVSevarogCharacter::OnAttackMontageBlendingOut(UAnimMontage* /*InMontage*/, bool /*bInterrupted*/)
{
	if (bIsComboChaining)
	{
		bIsComboChaining = false;
		return;
	}

	CombatStateComponent->RemoveState(ERVCombatState::Attacking);
	ActiveComboMontages.Empty();
	ActiveComboIndex = 0;
	OnAttackFinished.Broadcast();
}

//--- ForceEnd ----------------------------------------------------------------

void ARVSevarogCharacter::ForceEndCurrentAction()
{
	bIsComboChaining = false;
	ActiveComboMontages.Empty();
	ActiveComboIndex = 0;

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (IsValid(AnimInst))
	{
		AnimInst->Montage_Stop(0.2f);
	}

	CombatStateComponent->RemoveState(ERVCombatState::Attacking);
}

//--- Phase Attack ------------------------------------------------------------

bool ARVSevarogCharacter::ExecutePhaseAttack()
{
	if (bIsGroggy)     { return false; }
	if (IsAttacking()) { return false; }

	const FRVBossPhaseAttacks* PhaseAttacks = nullptr;
	switch (CurrentPhase)
	{
	case ERVBossPhase::Phase1: PhaseAttacks = &SevarogData->Phase1Attacks; break;
	case ERVBossPhase::Phase2: PhaseAttacks = &SevarogData->Phase2Attacks; break;
	}

	if (!PhaseAttacks || PhaseAttacks->Patterns.IsEmpty()) { return false; }

	const int32 Index = SelectWeightedPattern(PhaseAttacks->Patterns);
	StartComboChain(PhaseAttacks->Patterns[Index].ComboMontages);
	return true;
}

bool ARVSevarogCharacter::ExecuteRushAttack()
{
	if (bIsGroggy)     { return false; }
	if (IsAttacking()) { return false; }
	if (!IsValid(SevarogData->RushAttackMontage)) { return false; }

	StartComboChain({ SevarogData->RushAttackMontage });
	return true;
}

bool ARVSevarogCharacter::IsAttacking() const
{
	return CombatStateComponent->HasState(ERVCombatState::Attacking);
}

//--- Soul Siphon -------------------------------------------------------------

bool ARVSevarogCharacter::ExecuteSoulSiphon()
{
	if (bIsGroggy)     { return false; }
	if (IsAttacking()) { return false; }
	if (!IsValid(SevarogData->SoulSiphonMontage)) { return false; }

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInst)) { return false; }

	CombatStateComponent->AddState(ERVCombatState::Attacking);
	AnimInst->Montage_Play(SevarogData->SoulSiphonMontage);

	FOnMontageBlendingOutStarted BlendOutDelegate;
	BlendOutDelegate.BindUObject(this, &ARVSevarogCharacter::OnSoulSiphonBlendingOut);
	AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, SevarogData->SoulSiphonMontage);
	return true;
}

void ARVSevarogCharacter::OnSoulSiphonBlendingOut(UAnimMontage* /*InMontage*/, bool /*bInterrupted*/)
{
	CombatStateComponent->RemoveState(ERVCombatState::Attacking);
	OnAttackFinished.Broadcast();
}

//--- Subjugation -------------------------------------------------------------

bool ARVSevarogCharacter::ExecuteSubjugation()
{
	if (bIsGroggy)     { return false; }
	if (IsAttacking()) { return false; }
	if (!IsValid(SevarogData->SubjugationMontage)) { return false; }

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInst)) { return false; }

	CombatStateComponent->AddState(ERVCombatState::Attacking);
	AnimInst->Montage_Play(SevarogData->SubjugationMontage);

	FOnMontageBlendingOutStarted BlendOutDelegate;
	BlendOutDelegate.BindUObject(this, &ARVSevarogCharacter::OnSubjugationMontageBlendingOut);
	AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, SevarogData->SubjugationMontage);
	return true;
}

void ARVSevarogCharacter::OnSubjugationMontageBlendingOut(UAnimMontage* /*InMontage*/, bool /*bInterrupted*/)
{
	CombatStateComponent->RemoveState(ERVCombatState::Attacking);
	OnAttackFinished.Broadcast();
}

void ARVSevarogCharacter::SpawnSubjugationBlast()
{
	if (!IsValid(SevarogData)) { return; }

	TArray<AActor*> HitActors;
	UKismetSystemLibrary::SphereOverlapActors(
		this,
		GetActorLocation(),
		SevarogData->SubjugationBlastRadius,
		TArray<TEnumAsByte<EObjectTypeQuery>>{ UEngineTypes::ConvertToObjectType(ECC_Pawn) },
		nullptr,
		TArray<AActor*>{ this },
		HitActors);

	for (AActor* HitActor : HitActors)
	{
		if (IRVDamageable* Target = Cast<IRVDamageable>(HitActor))
		{
			FRVHitInfo HitInfo;
			HitInfo.Damage       = SevarogData->SubjugationBlastDamage;
			HitInfo.PoiseDamage  = SevarogData->SubjugationBlastPoiseDamage;
			HitInfo.HitType      = ERVHitType::Normal;
			HitInfo.HitDirection = (HitActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
			HitInfo.Instigator   = this;
			Target->ApplyDamage(HitInfo);
		}
	}

	// TODO: Spawn P_Sevarog_Subjugate_Blast Niagara at GetActorLocation()
}

//--- Groggy — 3-stage: StunStart → StunLoop → StunEnd -----------------------

void ARVSevarogCharacter::StartGroggy()
{
	if (bIsGroggy) { return; }
	
	bIsGroggy = true;
	CurrentPoiseDepletionCount = 0;

	if (IsAttacking())
	{
		ForceEndCurrentAction();
	}

	CombatStateComponent->AddState(ERVCombatState::Groggy);
	OnBossGroggyStarted.Broadcast();

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInst)) { return; }

	UAnimMontage* StunStartMontage = SevarogData->HitReactionAnimData->GroggyStunStartMontage;
	if (!IsValid(StunStartMontage)) { return; }

	AnimInst->Montage_Play(StunStartMontage);

	FOnMontageBlendingOutStarted BlendOutDelegate;
	BlendOutDelegate.BindUObject(this, &ARVSevarogCharacter::OnStunStartMontageBlendingOut);
	AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, StunStartMontage);
}

void ARVSevarogCharacter::OnStunStartMontageBlendingOut(UAnimMontage* /*InMontage*/, bool bInterrupted)
{
	if (bInterrupted || !bIsGroggy) { return; }

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInst)) { return; }

	UAnimMontage* StunLoopMontage = SevarogData->HitReactionAnimData->GroggyStunLoopMontage;
	if (!IsValid(StunLoopMontage)) { return; }

	AnimInst->Montage_Play(StunLoopMontage);

	// GroggyDuration timer starts when the loop begins — EndGroggy stops the loop.
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

	GetWorldTimerManager().ClearTimer(GroggyTimerHandle);

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInst)) { return; }

	UAnimMontage* StunLoopMontage = SevarogData->HitReactionAnimData->GroggyStunLoopMontage;
	if (IsValid(StunLoopMontage))
	{
		AnimInst->Montage_Stop(0.2f, StunLoopMontage);
	}

	UAnimMontage* StunEndMontage = SevarogData->HitReactionAnimData->GroggyStunEndMontage;
	if (!IsValid(StunEndMontage))
	{
		// No recovery montage assigned — clear state immediately.
		bIsGroggy = false;
		CombatStateComponent->RemoveState(ERVCombatState::Groggy);
		OnBossGroggyEnded.Broadcast();
		return;
	}

	AnimInst->Montage_Play(StunEndMontage);

	FOnMontageBlendingOutStarted BlendOutDelegate;
	BlendOutDelegate.BindUObject(this, &ARVSevarogCharacter::OnStunEndMontageBlendingOut);
	AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, StunEndMontage);
}

void ARVSevarogCharacter::OnStunEndMontageBlendingOut(UAnimMontage* /*InMontage*/, bool /*bInterrupted*/)
{
	bIsGroggy = false;
	CombatStateComponent->RemoveState(ERVCombatState::Groggy);
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