#include "Character/Enemy/RVSevarogCharacter.h"
#include "AI/RVAIController.h"
#include "Animation/AnimInstance.h"
#include "BrainComponent.h"
#include "Component/Attribute/RVVitalComponent.h"
#include "Component/Combat/RVHitReactionComponent.h"
#include "Components/CapsuleComponent.h"
#include "Data/Asset/RVSevarogDataAsset.h"
#include "Data/Asset/RVHitReactionAnimDataAsset.h"
#include "Data/Row/RVBossStatRow.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interface/RVDamageable.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

ARVSevarogCharacter::ARVSevarogCharacter()
{
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = ARVAIController::StaticClass();
}

void ARVSevarogCharacter::InitStats()
{
    if (!ensureMsgf(IsValid(SevarogData),
        TEXT("[%s] SevarogData must be assigned"), *GetNameSafe(this))) { return; }

    ensureMsgf(IsValid(SevarogData->HitReactionAnimData),
        TEXT("[%s] SevarogData.HitReactionAnimData not assigned"), *GetNameSafe(this));

    const FRVBossStatRow* StatRow = SevarogData->GetStatRow();
    if (!ensureMsgf(StatRow,
        TEXT("[%s] StatRow resolve failed — check DT_BossStats row name"), *GetNameSafe(this))) { return; }

    VitalComponent->InitFromStatRow(*StatRow);

    HitReactionComponent->InitParams(
        GetHitReactionAnimData(),
        StatRow->StaggerDuration,
        StatRow->StaggerThreshold,
        StatRow->KnockdownThreshold);

    NormalWalkSpeed      = StatRow->MoveSpeed;
    CachedGroggyDuration = StatRow->GroggyDuration;
    GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;

    SetCombatStat(StatRow->BaseDamage, StatRow->BasePoiseDamage, StatRow->AttackRadius);
}

void ARVSevarogCharacter::BeginPlay()
{
    Super::BeginPlay();

    HitReactionComponent->SetHitReactCapability(ERVHitReactCapability::Groggy);
    HitReactionComponent->OnGroggySequenceCompleted.AddUObject(
        this, &ARVSevarogCharacter::OnGroggySequenceCompleted);
    OnHitConfirmed.AddUObject(this, &ARVSevarogCharacter::OnHitConfirmedHandler);

    VitalComponent->OnHealthChanged.AddUObject(this, &ARVSevarogCharacter::CheckPhaseTransition);
    VitalComponent->OnPoiseDepleted.AddUObject(this, &ARVSevarogCharacter::OnPoiseDepleted);

    if (IsValid(SevarogData->MeleeTrailEffect))
    {
        MeleeTrailNC = UNiagaraFunctionLibrary::SpawnSystemAttached(
            SevarogData->MeleeTrailEffect,
            GetMesh(),
            TEXT("WeaponTip"),
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            false);

        if (IsValid(MeleeTrailNC))
        {
            MeleeTrailNC->SetVariableFloat(TEXT("Width"), SevarogData->MeleeTrailWidth);
            MeleeTrailNC->DeactivateImmediate();
        }
    }
}

URVHitReactionAnimDataAsset* ARVSevarogCharacter::GetHitReactionAnimData() const
{
    return SevarogData->HitReactionAnimData;
}

//--- State queries -----------------------------------------------------------

bool ARVSevarogCharacter::IsInHitReaction() const { return HasCombatState(ERVCombatState::HitReaction); }
bool ARVSevarogCharacter::IsKnockedDown()   const { return HasCombatState(ERVCombatState::Knockdown); }
bool ARVSevarogCharacter::IsAttacking()     const { return HasCombatState(ERVCombatState::Attacking); }

float ARVSevarogCharacter::GetStaggerDirectionForAnim() const
{
    return HitReactionComponent->GetStaggerDirection();
}

//--- Death -------------------------------------------------------------------

void ARVSevarogCharacter::OnDeath()
{
    ARVAIController* AICtrl = Cast<ARVAIController>(GetController());
    if (IsValid(AICtrl))
    {
        if (UBrainComponent* Brain = AICtrl->GetBrainComponent())
        {
            Brain->StopLogic(TEXT("Dead"));
        }
        AICtrl->StopMovement();
    }

    ForceEndCurrentAction();

    if (IsGroggy())
    {
        HitReactionComponent->AbortGroggy();
        RemoveCombatState(ERVCombatState::Groggy);
    }

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    URVHitReactionAnimDataAsset* HitReactionData = GetHitReactionAnimData();
    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    if (IsValid(AnimInst) && IsValid(HitReactionData) && IsValid(HitReactionData->DeathMontage))
    {
        StartDissolve();
        AnimInst->Montage_Stop(0.1f);
        AnimInst->Montage_Play(HitReactionData->DeathMontage);

        FOnMontageBlendingOutStarted BlendOutDelegate;
        BlendOutDelegate.BindUObject(this, &ARVSevarogCharacter::OnDeathMontageBlendingOut);
        AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, HitReactionData->DeathMontage);
    }
    else
    {
        bDeathMontageBlendedOut = true;
        StartDissolve();
    }

    OnBossDefeated.Broadcast();
}

void ARVSevarogCharacter::OnDeathMontageBlendingOut(UAnimMontage*, bool)
{
    bDeathMontageBlendedOut = true;
    TryDestroyActor();
}

//--- Dissolve ----------------------------------------------------------------

void ARVSevarogCharacter::StartDissolve()
{
    UWorld* World = GetWorld();

    DissolveStartTime = World->GetTimeSeconds();

    USkeletalMeshComponent* SkelMesh = GetMesh();
    const int32 NumMaterials = SkelMesh->GetNumMaterials();
    DissolveMIDs.SetNum(NumMaterials);
    for (int32 i = 0; i < NumMaterials; ++i)
    {
        DissolveMIDs[i] = SkelMesh->CreateDynamicMaterialInstance(i);
    }

    constexpr float TickInterval = 1.f / 30.f;
    World->GetTimerManager().SetTimer(
        DissolveTimerHandle, this, &ARVSevarogCharacter::TickDissolve, TickInterval, true);
}

void ARVSevarogCharacter::TickDissolve()
{
    UWorld* World = GetWorld();
    const float Alpha = FMath::Clamp(
        (World->GetTimeSeconds() - DissolveStartTime) / SevarogData->DissolveDuration, 0.f, 1.f);

    static const FName FadeOutParam(TEXT("FadeOut"));
    for (UMaterialInstanceDynamic* MID : DissolveMIDs)
    {
        if (IsValid(MID)) { MID->SetScalarParameterValue(FadeOutParam, Alpha); }
    }

    if (Alpha >= 1.f)
    {
        World->GetTimerManager().ClearTimer(DissolveTimerHandle);
        bDissolveCompleted = true;
        TryDestroyActor();
    }
}

void ARVSevarogCharacter::TryDestroyActor()
{
    if (bDissolveCompleted && bDeathMontageBlendedOut)
    {
        SetLifeSpan(0.1f);
    }
}

//--- BT task interface -------------------------------------------------------

bool ARVSevarogCharacter::ExecutePhaseAttack()
{
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
    if (!IsValid(SevarogData->RushAttackMontage)) { return false; }

    StartComboChain({ SevarogData->RushAttackMontage });
    return true;
}

bool ARVSevarogCharacter::ExecuteSoulSiphon()
{
    RotateToFacePlayer(ResolvePlayerPawn());
    return PlaySingleShotAction(SevarogData->SoulSiphon.Montage);
}

bool ARVSevarogCharacter::ExecuteSubjugation()
{
    RotateToFacePlayer(ResolvePlayerPawn());
    return PlaySingleShotAction(SevarogData->Subjugation.Montage);
}

bool ARVSevarogCharacter::PlaySingleShotAction(UAnimMontage* InMontage)
{
    if (!IsValid(InMontage)) { return false; }

    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    if (!ensureMsgf(IsValid(AnimInst),
        TEXT("[%s] PlaySingleShotAction: AnimInstance missing"),
        *GetNameSafe(this))) { return false; }

    AddCombatState(ERVCombatState::Attacking);
    AnimInst->Montage_Play(InMontage);

    FOnMontageBlendingOutStarted BlendOutDelegate;
    BlendOutDelegate.BindUObject(this, &ARVSevarogCharacter::OnSingleShotActionBlendingOut);
    AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, InMontage);
    return true;
}

void ARVSevarogCharacter::ForceEndCurrentAction()
{
    bIsComboChaining = false;
    ActiveComboMontages.Empty();
    ActiveComboIndex = 0;

    GetWorldTimerManager().ClearTimer(SubjugationDamageTimerHandle);
    PendingSubjugationLocations.Empty();
    PendingSwirlsSFX = nullptr;

    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    AnimInst->Montage_Stop(0.2f);

    RemoveCombatState(ERVCombatState::Attacking);
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
    UAnimMontage* Montage = ActiveComboMontages[InIndex];

    if (!IsValid(Montage)) { return; }

    AddCombatState(ERVCombatState::Attacking);
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

void ARVSevarogCharacter::OnAttackMontageBlendingOut(UAnimMontage*, bool bInterrupted)
{
    if (bIsComboChaining)
    {
        bIsComboChaining = false;
        return;
    }

    const bool bIsLastInChain = ActiveComboMontages.IsEmpty() ||
        (ActiveComboIndex >= ActiveComboMontages.Num() - 1);

    RemoveCombatState(ERVCombatState::Attacking);
    ActiveComboMontages.Empty();
    ActiveComboIndex = 0;

    if (!bInterrupted && bIsLastInChain) { OnAttackFinished.Broadcast(); }
}

void ARVSevarogCharacter::OnSingleShotActionBlendingOut(UAnimMontage*, bool bInterrupted)
{
    RemoveCombatState(ERVCombatState::Attacking);
    if (!bInterrupted) { OnAttackFinished.Broadcast(); }
}

//--- Groggy ------------------------------------------------------------------

void ARVSevarogCharacter::StartGroggy()
{
    if (IsAttacking()) { ForceEndCurrentAction(); }

    AddCombatState(ERVCombatState::Groggy);
    OnBossGroggyStarted.Broadcast();

    HitReactionComponent->TriggerGroggy(CachedGroggyDuration);
}

void ARVSevarogCharacter::EndGroggy()
{
    if (!IsGroggy()) { return; }
    HitReactionComponent->EndGroggy();
}

void ARVSevarogCharacter::OnGroggySequenceCompleted()
{
    RemoveCombatState(ERVCombatState::Groggy);
    ResetPoise();
}

void ARVSevarogCharacter::OnPoiseDepleted()
{
    if (IsGroggy()) { return; }
    StartGroggy();
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
    const FVector HitCenter = GetForwardLocation(SevarogData->SoulSiphon.HitForwardOffset);

    const APawn* Player       = ResolvePlayerPawn();
    const FVector OverrideDir = IsValid(Player)
        ? (GetActorLocation() - Player->GetActorLocation()).GetSafeNormal2D()
        : -GetActorForwardVector();

    ApplyForwardCapsuleDamageAt(HitCenter,
        SevarogData->SoulSiphon.HitRadius,
        SevarogData->SoulSiphon.HitHalfHeight,
        SevarogData->SoulSiphon.HitDamage,
        SevarogData->SoulSiphon.HitPoiseDamage,
        SevarogData->SoulSiphon.ImpactFX,
        OverrideDir);
}

//--- Subjugation -------------------------------------------------------------

void ARVSevarogCharacter::InitSubjugationLocations(UParticleSystem* InCastFX)
{
    const FVector Origin      = GetGroundOrigin();
    const float MinSeparation = SevarogData->Subjugation.SwirlDamageRadius * 2.f;

    PendingSubjugationLocations = GenerateSwirlLocations(
        Origin, SevarogData->Subjugation.SwirlSpreadRadius, MinSeparation, 3);

    for (const FVector& Loc : PendingSubjugationLocations)
    {
        SpawnFXAtLocation(InCastFX, Loc);
    }
}

void ARVSevarogCharacter::SpawnSubjugationBlast(UParticleSystem* InSwirlsFX, USoundBase* InSwirlsSFX)
{
    if (PendingSubjugationLocations.IsEmpty()) { return; }

    PendingSwirlsSFX = InSwirlsSFX;

    constexpr float DamageDelay = 0.5f;

    for (const FVector& SwirlLocation : PendingSubjugationLocations)
    {
        SpawnFXAtLocation(InSwirlsFX, SwirlLocation);
    }

    GetWorldTimerManager().SetTimer(
        SubjugationDamageTimerHandle,
        this,
        &ARVSevarogCharacter::ApplySubjugationDamage,
        FMath::Max(DamageDelay, KINDA_SMALL_NUMBER),
        false);
}

void ARVSevarogCharacter::ApplySubjugationDamage()
{
    for (const FVector& SwirlLocation : PendingSubjugationLocations)
    {
        if (IsValid(PendingSwirlsSFX))
        {
            UGameplayStatics::PlaySoundAtLocation(this, PendingSwirlsSFX, SwirlLocation);
        }

        ApplySphereDamageAt(SwirlLocation,
            SevarogData->Subjugation.SwirlDamageRadius,
            SevarogData->Subjugation.BlastDamage,
            SevarogData->Subjugation.BlastPoiseDamage);
    }

    PendingSubjugationLocations.Empty();
    PendingSwirlsSFX = nullptr;
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
                FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0.f);

            bool bTooClose = false;
            for (const FVector& Placed : Result)
            {
                if (FVector::Dist2D(Try, Placed) < InMinSeparation) { bTooClose = true; break; }
            }

            if (!bTooClose) { Candidate = Try; break; }
        }

        Result.Add(Candidate);
    }

    return Result;
}

//--- Damage helpers ----------------------------------------------------------

void ARVSevarogCharacter::ApplyDamageToOverlapResults(const TArray<FOverlapResult>& InOverlaps,
    float InDamage, float InPoiseDamage,
    UParticleSystem* InHitFX,
    const FVector& InOverrideDirection)
{
    TArray<AActor*> DamageTargets;
    for (const FOverlapResult& Overlap : InOverlaps)
    {
        if (AActor* Actor = Overlap.GetActor()) { DamageTargets.AddUnique(Actor); }
    }

    for (AActor* HitActor : DamageTargets)
    {
        if (IRVDamageable* Target = Cast<IRVDamageable>(HitActor))
        {
            FRVHitInfo HitInfo;
            HitInfo.Damage      = InDamage;
            HitInfo.PoiseDamage = InPoiseDamage;
            const FVector RawDir = InOverrideDirection.IsNearlyZero()
                ? (GetActorLocation() - HitActor->GetActorLocation())
                : InOverrideDirection;
            HitInfo.HitDirection = FVector(RawDir.X, RawDir.Y, 0.f).GetSafeNormal();
            HitInfo.Instigator   = this;

            const bool bDamaged = Target->ApplyDamage(HitInfo);
            if (bDamaged && IsValid(InHitFX))
            {
                SpawnFXAtLocation(InHitFX, HitActor->GetActorLocation());
            }
        }
    }
}

void ARVSevarogCharacter::ApplySphereDamageAt(const FVector& InLocation, float InRadius,
    float InDamage, float InPoiseDamage, UParticleSystem* InHitFX,
    const FVector& InOverrideDirection)
{
    FCollisionObjectQueryParams ObjectQueryParams;
    ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    TArray<FOverlapResult> Overlaps;
    GetWorld()->OverlapMultiByObjectType(
        Overlaps, InLocation, FQuat::Identity, ObjectQueryParams,
        FCollisionShape::MakeSphere(InRadius),
        QueryParams);

    ApplyDamageToOverlapResults(Overlaps, InDamage, InPoiseDamage, InHitFX, InOverrideDirection);
}

void ARVSevarogCharacter::ApplyForwardCapsuleDamageAt(const FVector& InLocation,
    float InRadius, float InHalfHeight,
    float InDamage, float InPoiseDamage, UParticleSystem* InHitFX,
    const FVector& InOverrideDirection)
{
    const float ClampedHalfHeight = FMath::Max(InHalfHeight, InRadius);
    const FQuat ForwardRot        = FQuat::FindBetweenNormals(FVector::UpVector, GetActorForwardVector());

    FCollisionObjectQueryParams ObjectQueryParams;
    ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    TArray<FOverlapResult> Overlaps;
    GetWorld()->OverlapMultiByObjectType(
        Overlaps, InLocation, ForwardRot,
        ObjectQueryParams,
        FCollisionShape::MakeCapsule(InRadius, ClampedHalfHeight),
        QueryParams);

    ApplyDamageToOverlapResults(Overlaps, InDamage, InPoiseDamage, InHitFX, InOverrideDirection);
}

//--- Hit FX ------------------------------------------------------------------

void ARVSevarogCharacter::OnHitConfirmedHandler(FVector ImpactLocation)
{
    SpawnFXAtLocation(SevarogData->MeleeHitImpact, ImpactLocation);

    if (IsValid(SevarogData->MeleeHitSFX))
    {
        UGameplayStatics::PlaySoundAtLocation(this, SevarogData->MeleeHitSFX, ImpactLocation);
    }
}

//--- Weapon trail ------------------------------------------------------------

void ARVSevarogCharacter::ActivateWeaponTrail()
{
    if (IsValid(MeleeTrailNC)) { MeleeTrailNC->Activate(true); }
}

void ARVSevarogCharacter::DeactivateWeaponTrail()
{
    if (IsValid(MeleeTrailNC)) { MeleeTrailNC->DeactivateImmediate(); }
}

//--- VFX helpers -------------------------------------------------------------

void ARVSevarogCharacter::SpawnFXAtLocation(UParticleSystem* InFX,
    const FVector& InLocation, const FRotator& InRotation, const FVector& InScale) const
{
    if (!IsValid(InFX)) { return; }
    UGameplayStatics::SpawnEmitterAtLocation(this, InFX, InLocation, InRotation, InScale);
}

//--- Phase transition --------------------------------------------------------

void ARVSevarogCharacter::SetBossPhase(ERVBossPhase InNewPhase)
{
    if (CurrentPhase == InNewPhase) { return; }
    CurrentPhase = InNewPhase;
    OnBossPhaseChanged.Broadcast(CurrentPhase);
}

void ARVSevarogCharacter::CheckPhaseTransition(float InNewHealthRatio)
{
    if (CurrentPhase != ERVBossPhase::Phase1) { return; }

    if (InNewHealthRatio <= SevarogData->Phase2Threshold)
    {
        VitalComponent->OnHealthChanged.RemoveAll(this);
        SetBossPhase(ERVBossPhase::Phase2);
    }
}

//--- Internal helpers --------------------------------------------------------

APawn* ARVSevarogCharacter::ResolvePlayerPawn() const
{
    const ARVAIController* AICtrl = Cast<ARVAIController>(GetController());
    return AICtrl ? AICtrl->GetPlayerPawn() : nullptr;
}

void ARVSevarogCharacter::RotateToFacePlayer(const APawn* InPlayer)
{
    if (!IsValid(InPlayer)) { return; }

    const FVector ToPlayer = (InPlayer->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
    if (ToPlayer.IsNearlyZero()) { return; }

    const FRotator CurrentRotation = GetActorRotation();
    const float DeltaYaw   = FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, ToPlayer.Rotation().Yaw);
    const float ClampedYaw = FMath::Clamp(DeltaYaw,
        -SevarogData->MaxComboTurnDegrees, SevarogData->MaxComboTurnDegrees);

    SetActorRotation(FRotator(0.f, CurrentRotation.Yaw + ClampedYaw, 0.f));
}
