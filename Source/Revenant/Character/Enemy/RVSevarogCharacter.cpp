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
#include "Particles/ParticleSystem.h"

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

    CombatStateComponent->SetCombatStat(
        SevarogData->BaseDamage,
        SevarogData->BasePoiseDamage,
        SevarogData->AttackRadius);
}

void ARVSevarogCharacter::BeginPlay()
{
    Super::BeginPlay(); // calls InitStats()

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
    if (bIsGroggy || IsAttacking())               { return false; }
    if (!IsValid(SevarogData->RushAttackMontage)) { return false; }

    StartComboChain({ SevarogData->RushAttackMontage });
    return true;
}

bool ARVSevarogCharacter::ExecuteSoulSiphon()
{
    // Cosmetic FX (cast trails, hand glow, body swirls) are placed as
    // AnimNotify_SpawnFX / AnimNotifyState_LoopFX on AM_Boss_SoulSiphon.
    RotateToFacePlayer(ResolvePlayerPawn());
    return PlaySingleShotAction(SevarogData->SoulSiphon.Montage);
}

bool ARVSevarogCharacter::ExecuteSubjugation()
{
    // Cosmetic cast FX is placed as AnimNotify_SpawnFX on AM_Boss_Subjugation.
    RotateToFacePlayer(ResolvePlayerPawn());
    return PlaySingleShotAction(SevarogData->Subjugation.Montage);
}

void ARVSevarogCharacter::ForceEndCurrentAction()
{
    bIsComboChaining = false;
    ActiveComboMontages.Empty();
    ActiveComboIndex = 0;

    // Stops all montages — intentional hard stop, not a graceful blend.
    // AnimNotifyState_LoopFX.NotifyEnd fires automatically on montage stop,
    // so looping cast FX are cleaned up without explicit tracking here.
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
        // Next montage in the chain is already playing — Attacking state and
        // ActiveComboMontages remain valid until the chain ends normally.
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
    if (bIsGroggy || IsAttacking()) { return false; }
    if (!IsValid(InMontage))        { return false; }

    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    if (!IsValid(AnimInst))         { return false; }

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
    // Called from UAnimNotify_SoulSiphonHit — can fire during edge-case blend-out.
    if (!IsValid(SevarogData)) { return; }

    const FVector HitCenter = GetForwardLocation(SevarogData->SoulSiphon.HitForwardOffset);

    ApplyRadialDamageAt(HitCenter,
        SevarogData->SoulSiphon.HitRadius,
        SevarogData->SoulSiphon.HitDamage,
        SevarogData->SoulSiphon.HitPoiseDamage,
        SevarogData->SoulSiphon.ImpactFX,
        GetActorForwardVector(), // forward thrust — player always knocked backward
        true);
}

//--- Subjugation blast -------------------------------------------------------

void ARVSevarogCharacter::SpawnSubjugationBlast()
{
    // Called from UAnimNotify_SubjugationBlast — can fire during edge-case blend-out.
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
        DrawDebugSphere(GetWorld(), InLocation,
            InRadius, 16, FColor::Cyan, false, 2.f);
    }
#endif

    for (AActor* HitActor : HitActors)
    {
        if (IRVDamageable* Target = Cast<IRVDamageable>(HitActor))
        {
            FRVHitInfo HitInfo;
            HitInfo.Damage       = InDamage;
            HitInfo.PoiseDamage  = InPoiseDamage;
            HitInfo.HitDirection = InOverrideDirection.IsNearlyZero()
                ? (HitActor->GetActorLocation() - InLocation).GetSafeNormal()
                : InOverrideDirection;
            HitInfo.Instigator   = this;

            const bool bDamaged = Target->ApplyDamage(HitInfo);
            if (bDamaged && IsValid(InHitFX))
            {
                SpawnFXAtLocation(InHitFX, HitActor->GetActorLocation());
            }
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