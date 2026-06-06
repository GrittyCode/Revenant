#include "Game/RVBossEncounterVolume.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "Controller/RVPlayerController.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

ARVBossEncounterVolume::ARVBossEncounterVolume()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ARVBossEncounterVolume::BeginPlay()
{
    Super::BeginPlay();
    OnActorBeginOverlap.AddDynamic(this, &ARVBossEncounterVolume::OnOverlapBegin);

    CachedPlayerController = Cast<ARVPlayerController>(GetWorld()->GetFirstPlayerController());

    if (IsValid(CachedPlayerController))
    {
        OnBossSpawned.AddUObject(CachedPlayerController, &ARVPlayerController::OnBossSpawned);
    }
}

void ARVBossEncounterVolume::OnOverlapBegin(AActor* /*OverlappedActor*/, AActor* OtherActor)
{
    if (bTriggered) { return; }

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!IsValid(PC)) { return; }
    if (OtherActor != PC->GetPawn()) { return; }

    bTriggered = true;
    BeginBossEncounter();
}

void ARVBossEncounterVolume::BeginBossEncounter()
{
    if (!ensureMsgf(IsValid(BossCharacterClass),
        TEXT("[ARVBossEncounterVolume] BossCharacterClass is not assigned"))) { return; }

    const FTransform SpawnTransform = IsValid(BossSpawnPoint)
        ? BossSpawnPoint->GetActorTransform()
        : GetActorTransform();

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    SpawnedBoss = GetWorld()->SpawnActor<ARVSevarogCharacter>(BossCharacterClass, SpawnTransform, Params);

    if (!ensureMsgf(IsValid(SpawnedBoss),
        TEXT("[ARVBossEncounterVolume] Failed to spawn boss — check BossCharacterClass and spawn location"))) { return; }

    SpawnedBoss->SetActorHiddenInGame(true);

    PauseBossAI();

    if (IsValid(CachedPlayerController))
    {
        CachedPlayerController->LockInputForCutscene();
        CachedPlayerController->SetHUDVisible(false);
    }

    StartCutscene();
}

void ARVBossEncounterVolume::StartCutscene()
{
    // No sequence assigned or sequence player unavailable — skip directly to combat.
    if (!IsValid(CutsceneSequenceActor))
    {
        OnCutsceneFinished();
        return;
    }

    ULevelSequencePlayer* SeqPlayer = CutsceneSequenceActor->GetSequencePlayer();
    if (!IsValid(SeqPlayer))
    {
        OnCutsceneFinished();
        return;
    }

    // Natural end: OnStop fires → OnCutsceneFinished().
    SeqPlayer->OnStop.AddDynamic(this, &ARVBossEncounterVolume::OnCutsceneFinished);

    // Skip input: subscribe while cutscene is live; unsubscribed in OnCutsceneFinished().
    if (IsValid(CachedPlayerController))
    {
        CutsceneSkipHandle = CachedPlayerController->OnCutsceneSkipRequested.AddUObject(
            this, &ARVBossEncounterVolume::SkipCutscene);
    }

    SeqPlayer->Play();

    if (IsValid(SpawnedBoss) && IsValid(BossIntroMontage))
    {
        SpawnedBoss->PlayAnimMontage(BossIntroMontage);
    }

    SpawnedBoss->SetActorHiddenInGame(false);

    if (IsValid(CutsceneBGM))
    {
        CutsceneBGMAudioComponent = UGameplayStatics::SpawnSound2D(GetWorld(), CutsceneBGM);
    }
}

void ARVBossEncounterVolume::SkipCutscene()
{
    if (!IsValid(CutsceneSequenceActor)) { return; }

    ULevelSequencePlayer* SeqPlayer = CutsceneSequenceActor->GetSequencePlayer();
    if (!IsValid(SeqPlayer) || !SeqPlayer->IsPlaying()) { return; }
	
	SpawnedBoss->StopAnimMontage();
	SeqPlayer->Stop();
}

void ARVBossEncounterVolume::OnCutsceneFinished()
{
    // Always unsubscribe first — prevents double-fire if skip and natural end race.
    if (IsValid(CachedPlayerController) && CutsceneSkipHandle.IsValid())
    {
        CachedPlayerController->OnCutsceneSkipRequested.Remove(CutsceneSkipHandle);
        CutsceneSkipHandle.Reset();
    }

    if (IsValid(CutsceneBGMAudioComponent))
    {
        CutsceneBGMAudioComponent->Stop();
    }

    if (IsValid(CachedPlayerController))
    {
        CachedPlayerController->SetHUDVisible(true);
        CachedPlayerController->UnlockInputAfterCutscene();
    }

    if (IsValid(CombatBGM))
    {
        UGameplayStatics::SpawnSound2D(GetWorld(), CombatBGM);
    }

    ResumeBossAI();
    OnBossSpawned.Broadcast(SpawnedBoss);
}

void ARVBossEncounterVolume::PauseBossAI()
{
    if (!IsValid(SpawnedBoss)) { return; }

    AAIController* AICon = Cast<AAIController>(SpawnedBoss->GetController());
    if (!IsValid(AICon)) { return; }

    AICon->StopMovement();
    if (UBrainComponent* Brain = AICon->GetBrainComponent())
    {
        Brain->StopLogic(TEXT("Cutscene"));
    }
}

void ARVBossEncounterVolume::ResumeBossAI()
{
    if (!IsValid(SpawnedBoss)) { return; }

    AAIController* AICon = Cast<AAIController>(SpawnedBoss->GetController());
    if (!IsValid(AICon)) { return; }

    if (UBrainComponent* Brain = AICon->GetBrainComponent())
    {
        Brain->RestartLogic();
    }
}