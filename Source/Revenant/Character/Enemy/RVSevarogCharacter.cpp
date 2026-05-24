#include "Character/Enemy/RVSevarogCharacter.h"
#include "AI/RVAIController.h"
#include "Animation/AnimInstance.h"
#include "BrainComponent.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVHitReactionComponent.h"
#include "Components/CapsuleComponent.h"
#include "Data/RVSevarogDataAsset.h"
#include "Data/RVHitReactionAnimDataAsset.h"
#include "Data/RVEnemyStatRow.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Interface/RVDamageable.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Particles/ParticleSystem.h"

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

    HitReactionComponent->SetHitReactCapability(ERVHitReactCapability::Groggy);
    HitReactionComponent->OnGroggySequenceCompleted.AddUObject(
        this, &ARVSevarogCharacter::OnGroggySequenceCompleted);

    AttributeComponent->OnHealthChanged.AddDynamic(this, &ARVSevarogCharacter::CheckPhaseTransition);
    AttributeComponent->OnPoiseDepleted.AddDynamic(this, &ARVSevarogCharacter::OnPoiseDepleted);
}

URVHitReactionAnimDataAsset* ARVSevarogCharacter::GetHitReactionAnimData() const
{
    return IsValid(SevarogData) ? SevarogData->HitReactionAnimData : nullptr;
}

//--- Death -------------------------------------------------------------------

void ARVSevarogCharacter::OnDeath()
{
    if (AAIController* AICtrl = Cast<AAIController>(GetController()))
    {
        if (UBrainComponent* Brain = AICtrl->GetBrainComponent())
        {
            Brain->StopLogic(TEXT("Dead"));
        }
        AICtrl->StopMovement();
    }

    ForceEndCurrentAction();

    if (bIsGroggy)
    {
        bIsGroggy = false;
        HitReactionComponent->AbortGroggy();
    }

    CombatStateComponent->RemoveState(ERVCombatState::Groggy);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    URVHitReactionAnimDataAsset* HitReactionData = GetHitReactionAnimData();
    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    if (IsValid(AnimInst) && IsValid(HitReactionData) && IsValid(HitReactionData->DeathMontage))
    {
        AnimInst->Montage_Stop(0.1f);
        AnimInst->Montage_Play(HitReactionData->DeathMontage);

        FOnMontageBlendingOutStarted BlendOutDelegate;
        BlendOutDelegate.BindUObject(this, &ARVSevarogCharacter::OnDeathMontageBlendingOut);
        AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, HitReactionData->DeathMontage);
    }
    else
    {
        SetLifeSpan(2.f);
    }

    OnBossDefeated.Broadcast();
}

void ARVSevarogCharacter::OnDeathMontageBlendingOut(UAnimMontage* /*InMontage*/, bool /*bInterrupted*/)
{
    SetLifeSpan(1.f);
}

//--- BT task interface -------------------------------------------------------

bool ARVSevarogCharacter::IsAttacking() const
{
    return CombatStateComponent->HasState(ERVCombatState::Attacking);
}

bool ARVSevarogCharacter::ExecutePhaseAttack()
{
    if (bIsGroggy || IsAttacking()) { return false; }

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
    if (bIsGroggy || IsAttacking())                       { return false; }
    if (!IsValid(SevarogData->RushAttackMontage))         { return false; }

    // Rotate is handled inside PlayComboMontageAt.
    StartComboChain({ SevarogData->RushAttackMontage });
    return true;
}

bool ARVSevarogCharacter::ExecuteSoulSiphon()
{
    RotateToFacePlayer(ResolvePlayerPawn());

    if (!PlaySingleShotAction(SevarogData->SoulSiphon.Montage)) { return false; }

    SpawnFXAttached(SevarogData->SoulSiphon.CastFX);
    SpawnFXAttached(SevarogData->SoulSiphon.BodySwirlsFX);
    SpawnFXAttached(SevarogData->SoulSiphon.CastTrailsFX);

    SpawnFXAtLocation(SevarogData->SoulSiphon.HandFX,
        GetForwardLocation(), GetActorRotation(), FVector(0.5f));

    return true;
}

bool ARVSevarogCharacter::ExecuteSubjugation()
{
    RotateToFacePlayer(ResolvePlayerPawn());

    if (!PlaySingleShotAction(SevarogData->Subjugation.Montage)) { return false; }

    SpawnFXAttached(SevarogData->Subjugation.CastFX);
    return true;
}

void ARVSevarogCharacter::ForceEndCurrentAction()
{
    bIsComboChaining = false;
    ActiveComboMontages.Empty();
    ActiveComboIndex = 0;

    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    if (IsValid(AnimInst)) { AnimInst->Montage_Stop(0.2f); }

    CombatStateComponent->RemoveState(ERVCombatState::Attacking);
}

//--- Combo chain -------------------------------------------------------------

int32 ARVSevarogCharacter::SelectWeightedPattern(const TArray<FRVBossAttackPattern>& InPatterns)
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
        if (Roll < 0) { return i; }
    }

    return InPatterns.Num() - 1;
}

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
    RotateToFacePlayer(ResolvePlayerPawn());

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

    bIsComboChaining = true;
    ++ActiveComboIndex;
    PlayComboMontageAt(ActiveComboIndex);
}

void ARVSevarogCharacter::OnAttackMontageBlendingOut(UAnimMontage* /*InMontage*/, bool bInterrupted)
{
    if (bIsComboChaining)
    {
        bIsComboChaining = false;
        return;
    }

    CombatStateComponent->RemoveState(ERVCombatState::Attacking);
    ActiveComboMontages.Empty();
    ActiveComboIndex = 0;

    if (!bInterrupted) { OnAttackFinished.Broadcast(); }
}

bool ARVSevarogCharacter::PlaySingleShotAction(UAnimMontage* InMontage)
{
    if (bIsGroggy || IsAttacking())  { return false; }
    if (!IsValid(InMontage))         { return false; }

    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst))          { return false; }

    CombatStateComponent->AddState(ERVCombatState::Attacking);
    AnimInst->Montage_Play(InMontage);

    FOnMontageBlendingOutStarted BlendOutDelegate;
    BlendOutDelegate.BindUObject(this, &ARVSevarogCharacter::OnSingleShotActionBlendingOut);
    AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, InMontage);
    return true;
}

void ARVSevarogCharacter::OnSingleShotActionBlendingOut(UAnimMontage* /*InMontage*/, bool bInterrupted)
{
    CombatStateComponent->RemoveState(ERVCombatState::Attacking);
    if (!bInterrupted) { OnAttackFinished.Broadcast(); }
}

//--- Groggy ------------------------------------------------------------------

void ARVSevarogCharacter::StartGroggy()
{
    if (bIsGroggy) { return; }

    bIsGroggy = true;

    if (IsAttacking()) { ForceEndCurrentAction(); }

    CombatStateComponent->AddState(ERVCombatState::Groggy);
    OnBossGroggyStarted.Broadcast();

    HitReactionComponent->TriggerGroggy(SevarogData->GroggyDuration);
}

void ARVSevarogCharacter::EndGroggy()
{
    if (!bIsGroggy) { return; }
    HitReactionComponent->EndGroggy();
}

void ARVSevarogCharacter::OnGroggySequenceCompleted()
{
    bIsGroggy = false;
    CombatStateComponent->RemoveState(ERVCombatState::Groggy);
    AttributeComponent->ResetPoise();
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

//--- Soul Siphon hit ---------------------------------------------------------

void ARVSevarogCharacter::ExecuteSoulSiphonHit()
{
    if (!IsValid(SevarogData)) { return; }

    const FVector HitCenter = GetForwardLocation(SevarogData->SoulSiphon.HitForwardOffset);

#if !UE_BUILD_SHIPPING
    DrawDebugSphere(GetWorld(), HitCenter,
        SevarogData->SoulSiphon.HitRadius, 16, FColor::Cyan, false, 2.f);
#endif

    ApplyRadialDamageAt(HitCenter,
        SevarogData->SoulSiphon.HitRadius,
        SevarogData->SoulSiphon.HitDamage,
        SevarogData->SoulSiphon.HitPoiseDamage);

    SpawnFXAtLocation(SevarogData->SoulSiphon.ImpactFX, HitCenter, GetActorRotation());
}

//--- Subjugation blast -------------------------------------------------------

void ARVSevarogCharacter::SpawnSubjugationBlast()
{
    if (!IsValid(SevarogData)) { return; }

    const FVector Origin = GetGroundOrigin();
    SpawnFXAtLocation(SevarogData->Subjugation.BlastFX, Origin);

    const float MinSeparation = SevarogData->Subjugation.SwirlDamageRadius * 2.f;
    const TArray<FVector> SwirlLocations = GenerateSwirlLocations(
        Origin,
        SevarogData->Subjugation.SwirlSpreadRadius,
        MinSeparation,
        3);

    for (const FVector& SwirlLocation : SwirlLocations)
    {
        SpawnFXAtLocation(SevarogData->Subjugation.SwirlsFX, SwirlLocation);

#if !UE_BUILD_SHIPPING
        DrawDebugSphere(GetWorld(), SwirlLocation,
            SevarogData->Subjugation.SwirlDamageRadius, 16, FColor::Orange, false, 2.f);
#endif

        ApplyRadialDamageAt(SwirlLocation,
            SevarogData->Subjugation.SwirlDamageRadius,
            SevarogData->Subjugation.BlastDamage,
            SevarogData->Subjugation.BlastPoiseDamage);
    }
}

TArray<FVector> ARVSevarogCharacter::GenerateSwirlLocations(const FVector& InOrigin,
    float InSpreadRadius, float InMinSeparation, int32 InCount)
{
    TArray<FVector> Result;
    Result.Reserve(InCount);

    static constexpr int32 MaxAttemptsPerPoint = 30;

    for (int32 i = 0; i < InCount; ++i)
    {
        FVector Candidate = InOrigin;

        for (int32 Attempt = 0; Attempt < MaxAttemptsPerPoint; ++Attempt)
        {
            const float Angle = FMath::RandRange(0.f, 2.f * PI);
            const float Dist  = FMath::Sqrt(FMath::FRand()) * InSpreadRadius;

            const FVector Try = InOrigin + FVector(
                FMath::Cos(Angle) * Dist,
                FMath::Sin(Angle) * Dist,
                0.f);

            bool bTooClose = false;
            for (const FVector& Placed : Result)
            {
                if (FVector::Dist2D(Try, Placed) < InMinSeparation)
                {
                    bTooClose = true;
                    break;
                }
            }

            if (!bTooClose)
            {
                Candidate = Try;
                break;
            }
        }

        Result.Add(Candidate);
    }

    return Result;
}

//--- Radial damage -----------------------------------------------------------

void ARVSevarogCharacter::ApplyRadialDamageAt(const FVector& InLocation, float InRadius,
    float InDamage, float InPoiseDamage)
{
    TArray<AActor*> HitActors;
    UKismetSystemLibrary::SphereOverlapActors(
        this,
        InLocation,
        InRadius,
        TArray<TEnumAsByte<EObjectTypeQuery>>{ UEngineTypes::ConvertToObjectType(ECC_Pawn) },
        nullptr,
        TArray<AActor*>{ this },
        HitActors);

    for (AActor* HitActor : HitActors)
    {
        if (IRVDamageable* Target = Cast<IRVDamageable>(HitActor))
        {
            FRVHitInfo HitInfo;
            HitInfo.Damage       = InDamage;
            HitInfo.PoiseDamage  = InPoiseDamage;
            HitInfo.HitType      = ERVHitType::Normal;
            HitInfo.HitDirection = (HitActor->GetActorLocation() - InLocation).GetSafeNormal();
            HitInfo.Instigator   = this;
            Target->ApplyDamage(HitInfo);
        }
    }
}

//--- VFX helpers -------------------------------------------------------------

void ARVSevarogCharacter::SpawnFXAtLocation(UParticleSystem* InFX,
    const FVector& InLocation, const FRotator& InRotation, const FVector& InScale) const
{
    if (!IsValid(InFX)) { return; }
    UGameplayStatics::SpawnEmitterAtLocation(this, InFX, InLocation, InRotation, InScale);
}

void ARVSevarogCharacter::SpawnFXAttached(UParticleSystem* InFX, FName InSocketName) const
{
    if (!IsValid(InFX)) { return; }
    UGameplayStatics::SpawnEmitterAttached(
        InFX,
        GetMesh(),
        InSocketName,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::SnapToTarget,
        true);
}

//--- Phase transition --------------------------------------------------------

void ARVSevarogCharacter::SetBossPhase(ERVBossPhase InNewPhase)
{
    if (CurrentPhase == InNewPhase) { return; }
    CurrentPhase = InNewPhase;
    OnBossPhaseChanged.Broadcast(CurrentPhase);
}

void ARVSevarogCharacter::CheckPhaseTransition(float /*InNewHealth*/, float /*InDelta*/)
{
    if (CurrentPhase == ERVBossPhase::Phase1
        && AttributeComponent->GetHealthPercent() <= SevarogData->Phase2Threshold)
    {
        SetBossPhase(ERVBossPhase::Phase2);
    }
}

void ARVSevarogCharacter::OnPoiseDepleted()
{
    if (bIsGroggy) { return; }
    StartGroggy();
}

//--- Internal helpers --------------------------------------------------------

APawn* ARVSevarogCharacter::ResolvePlayerPawn() const
{
    const ARVAIController* AICtrl = Cast<ARVAIController>(GetController());
    return IsValid(AICtrl) ? AICtrl->GetPlayerPawn() : nullptr;
}

void ARVSevarogCharacter::RotateToFacePlayer(const APawn* InPlayer)
{
    if (!IsValid(InPlayer) || !IsValid(SevarogData)) { return; }

    const FVector ToPlayer = (InPlayer->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
    if (ToPlayer.IsNearlyZero()) { return; }

    const FRotator CurrentRotation = GetActorRotation();
    const float DeltaYaw   = FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, ToPlayer.Rotation().Yaw);
    const float ClampedYaw = FMath::Clamp(DeltaYaw, -SevarogData->MaxComboTurnDegrees, SevarogData->MaxComboTurnDegrees);

    SetActorRotation(FRotator(0.f, CurrentRotation.Yaw + ClampedYaw, 0.f));
}