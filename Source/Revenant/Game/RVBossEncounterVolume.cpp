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

    // Cache once — PlayerController exists before level actors run BeginPlay.
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
    if (!IsValid(CutsceneSequenceActor))
    {
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
        return;
    }

    ULevelSequencePlayer* SeqPlayer = CutsceneSequenceActor->GetSequencePlayer();
    if (!IsValid(SeqPlayer))
    {
        ResumeBossAI();
        OnBossSpawned.Broadcast(SpawnedBoss);
        return;
    }

    SeqPlayer->OnStop.AddDynamic(this, &ARVBossEncounterVolume::OnCutsceneFinished);
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

void ARVBossEncounterVolume::OnCutsceneFinished()
{
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