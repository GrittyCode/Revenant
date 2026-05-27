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
#include "Data/RVCharacterStatRow.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Interface/RVDamageable.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
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
    SevarogData = Cast<URVSevarogDataAsset>(CharacterData);

    if (!ensureMsgf(IsValid(SevarogData),
        TEXT("[%s] CharacterData must be URVSevarogDataAsset"), *GetNameSafe(this))) { return; }

    ensureMsgf(IsValid(SevarogData->HitReactionAnimData),
        TEXT("[%s] SevarogData.HitReactionAnimData not assigned"), *GetNameSafe(this));

    const FRVCharacterStatRow* StatRow = SevarogData->GetStatRow();
    if (!ensureMsgf(StatRow,
        TEXT("[%s] StatRow resolve failed — check DT_EnemyStats row name"), *GetNameSafe(this))) { return; }

    AttributeComponent->InitFromStatRow(*StatRow);

    NormalWalkSpeed = StatRow->MoveSpeed;
    GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;

    SetCombatStat(SevarogData->BaseDamage, SevarogData->BasePoiseDamage, SevarogData->AttackRadius);
    SetHitFX(nullptr, SevarogData->MeleeHitImpact, SevarogData->MeleeHitSFX);
}

void ARVSevarogCharacter::BeginPlay()
{
    Super::BeginPlay();

    HitReactionComponent->SetHitReactCapability(ERVHitReactCapability::Groggy);
    HitReactionComponent->OnGroggySequenceCompleted.AddUObject(
        this, &ARVSevarogCharacter::OnGroggySequenceCompleted);

    AttributeComponent->OnHealthChanged.AddDynamic(this, &ARVSevarogCharacter::CheckPhaseTransition);
    AttributeComponent->OnPoiseDepleted.AddDynamic(this, &ARVSevarogCharacter::OnPoiseDepleted);

    if (IsValid(SevarogData) && IsValid(SevarogData->MeleeTrailEffect))
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

    RemoveCombatState(ERVCombatState::Groggy);
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
        StartDissolve();
    }

    OnBossDefeated.Broadcast();
}

void ARVSevarogCharacter::OnDeathMontageBlendingOut(UAnimMontage*, bool)
{
    if (!GetWorldTimerManager().IsTimerActive(DissolveTimerHandle))
    {
        SetLifeSpan(0.1f);
    }
}

//--- Dissolve ----------------------------------------------------------------

void ARVSevarogCharacter::StartDissolve()
{
    DissolveDuration = IsValid(SevarogData) ? SevarogData->DissolveDuration : 2.f;
    DissolveElapsed  = 0.f;

    USkeletalMeshComponent* SkelMesh = GetMesh();
    if (!IsValid(SkelMesh)) { SetLifeSpan(0.1f); return; }

    const int32 NumMaterials = SkelMesh->GetNumMaterials();
    DissolveMIDs.SetNum(NumMaterials);
    for (int32 i = 0; i < NumMaterials; ++i)
    {
        DissolveMIDs[i] = SkelMesh->CreateDynamicMaterialInstance(i);
    }

    constexpr float TickInterval = 1.f / 30.f;
    GetWorld()->GetTimerManager().SetTimer(
        DissolveTimerHandle, this, &ARVSevarogCharacter::TickDissolve, TickInterval, true);
}

void ARVSevarogCharacter::TickDissolve()
{
    constexpr float TickInterval = 1.f / 30.f;
    DissolveElapsed += TickInterval;

    const float Alpha = FMath::Clamp(DissolveElapsed / DissolveDuration, 0.f, 1.f);

    static const FName FadeOutParam(TEXT("FadeOut"));
    for (UMaterialInstanceDynamic* MID : DissolveMIDs)
    {
        if (IsValid(MID)) { MID->SetScalarParameterValue(FadeOutParam, Alpha); }
    }

    if (Alpha >= 1.f)
    {
        GetWorld()->GetTimerManager().ClearTimer(DissolveTimerHandle);
        SetLifeSpan(0.1f);
    }
}

//--- BT task interface -------------------------------------------------------

bool ARVSevarogCharacter::IsAttacking() const
{
    return HasCombatState(ERVCombatState::Attacking);
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
    if (bIsGroggy || IsAttacking())               { return false; }
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

void ARVSevarogCharacter::ForceEndCurrentAction()
{
    bIsComboChaining = false;
    ActiveComboMontages.Empty();
    ActiveComboIndex = 0;

    // Cancel pending subjugation damage timer and clear state.
    GetWorldTimerManager().ClearTimer(SubjugationDamageTimerHandle);
    PendingSubjugationLocations.Empty();
    PendingSwirlsSFX = nullptr;

    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    if (IsValid(AnimInst)) { AnimInst->Montage_Stop(0.2f); }

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
    if (!IsValid(AnimInst)) { return; }

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

    RemoveCombatState(ERVCombatState::Attacking);
    ActiveComboMontages.Empty();
    ActiveComboIndex = 0;

    if (!bInterrupted) { OnAttackFinished.Broadcast(); }
}

bool ARVSevarogCharacter::PlaySingleShotAction(UAnimMontage* InMontage)
{
    if (bIsGroggy || IsAttacking()) { return false; }
    if (!IsValid(InMontage))        { return false; }

    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst))         { return false; }

    AddCombatState(ERVCombatState::Attacking);
    AnimInst->Montage_Play(InMontage);

    FOnMontageBlendingOutStarted BlendOutDelegate;
    BlendOutDelegate.BindUObject(this, &ARVSevarogCharacter::OnSingleShotActionBlendingOut);
    AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, InMontage);
    return true;
}

void ARVSevarogCharacter::OnSingleShotActionBlendingOut(UAnimMontage*, bool bInterrupted)
{
    RemoveCombatState(ERVCombatState::Attacking);
    if (!bInterrupted) { OnAttackFinished.Broadcast(); }
}

//--- Groggy ------------------------------------------------------------------

void ARVSevarogCharacter::StartGroggy()
{
    if (bIsGroggy) { return; }

    bIsGroggy = true;

    if (IsAttacking()) { ForceEndCurrentAction(); }

    AddCombatState(ERVCombatState::Groggy);
    OnBossGroggyStarted.Broadcast();

    if (IsValid(SevarogData) && IsValid(SevarogData->GroggyStartSFX))
    {
        UGameplayStatics::PlaySoundAtLocation(this, SevarogData->GroggyStartSFX, GetActorLocation());
    }

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
    RemoveCombatState(ERVCombatState::Groggy);
    ResetPoise();
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

    ApplyRadialDamageAt(HitCenter,
        SevarogData->SoulSiphon.HitRadius,
        SevarogData->SoulSiphon.HitDamage,
        SevarogData->SoulSiphon.HitPoiseDamage,
        SevarogData->SoulSiphon.ImpactFX,
        GetActorForwardVector(),
        true);
}

//--- Subjugation -------------------------------------------------------------

void ARVSevarogCharacter::InitSubjugationLocations(UParticleSystem* InCastFX)
{
    if (!IsValid(SevarogData)) { return; }

    const FVector Origin      = GetGroundOrigin();
    const float MinSeparation = SevarogData->Subjugation.SwirlDamageRadius * 2.f;

    PendingSubjugationLocations = GenerateSwirlLocations(
        Origin, SevarogData->Subjugation.SwirlSpreadRadius, MinSeparation, 3);

    for (const FVector& Loc : PendingSubjugationLocations)
    {
        SpawnFXAtLocation(InCastFX, Loc);

#if !UE_BUILD_SHIPPING
        DrawDebugSphere(GetWorld(), Loc,
            SevarogData->Subjugation.SwirlDamageRadius, 16, FColor::Yellow, false, 1.5f);
#endif
    }
}

void ARVSevarogCharacter::SpawnSubjugationBlast(UParticleSystem* InSwirlsFX, USoundBase* InSwirlsSFX)
{
    if (!IsValid(SevarogData) || PendingSubjugationLocations.IsEmpty()) { return; }

    PendingSwirlsSFX = InSwirlsSFX;
	
	const float DamageDelay = 0.5f;
	
    for (const FVector& SwirlLocation : PendingSubjugationLocations)
    {
        SpawnFXAtLocation(InSwirlsFX, SwirlLocation);

#if !UE_BUILD_SHIPPING
        DrawDebugSphere(GetWorld(), SwirlLocation,
            SevarogData->Subjugation.SwirlDamageRadius, 16, FColor::Orange, false,
              DamageDelay + 1.f);
#endif
    }

    // Locations kept alive until ApplySubjugationDamage fires.
    GetWorldTimerManager().SetTimer(
        SubjugationDamageTimerHandle,
        this,
        &ARVSevarogCharacter::ApplySubjugationDamage,
        FMath::Max(DamageDelay, KINDA_SMALL_NUMBER),
        false);
}

void ARVSevarogCharacter::ApplySubjugationDamage()
{
    if (!IsValid(SevarogData)) { return; }

    for (const FVector& SwirlLocation : PendingSubjugationLocations)
    {
        if (IsValid(PendingSwirlsSFX))
        {
            UGameplayStatics::PlaySoundAtLocation(this, PendingSwirlsSFX, SwirlLocation);
        }

        ApplyRadialDamageAt(SwirlLocation,
            SevarogData->Subjugation.SwirlDamageRadius,
            SevarogData->Subjugation.BlastDamage,
            SevarogData->Subjugation.BlastPoiseDamage);
    }

    PendingSubjugationLocations.Empty();
    PendingSwirlsSFX = nullptr;
}

//--- Subjugation helpers -----------------------------------------------------

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

//--- Radial damage -----------------------------------------------------------

void ARVSevarogCharacter::ApplyRadialDamageAt(const FVector& InLocation, float InRadius,
    float InDamage, float InPoiseDamage, UParticleSystem* InHitFX,
    const FVector& InOverrideDirection, bool bUseCapsule)
{
    TArray<AActor*> HitActors;
    const TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes
        { UEngineTypes::ConvertToObjectType(ECC_Pawn) };
    const TArray<AActor*> IgnoreActors{ this };

    if (bUseCapsule)
    {
        UKismetSystemLibrary::CapsuleOverlapActors(
            this, InLocation, InRadius, InRadius,
            ObjectTypes, nullptr, IgnoreActors, HitActors);
    }
    else
    {
        UKismetSystemLibrary::SphereOverlapActors(
            this, InLocation, InRadius,
            ObjectTypes, nullptr, IgnoreActors, HitActors);
    }

#if !UE_BUILD_SHIPPING
    if (bUseCapsule)
    {
        DrawDebugCapsule(GetWorld(), InLocation,
            InRadius, InRadius, FQuat::Identity, FColor::Cyan, false, 2.f);
    }
    else
    {
        DrawDebugSphere(GetWorld(), InLocation, InRadius, 16, FColor::Cyan, false, 2.f);
    }
#endif

    for (AActor* HitActor : HitActors)
    {
        if (IRVDamageable* Target = Cast<IRVDamageable>(HitActor))
        {
            FRVHitInfo HitInfo;
            HitInfo.Damage      = InDamage;
            HitInfo.PoiseDamage = InPoiseDamage;
            const FVector RawDir = InOverrideDirection.IsNearlyZero()
                ? (HitActor->GetActorLocation() - InLocation)
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

void ARVSevarogCharacter::CheckPhaseTransition(float, float)
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