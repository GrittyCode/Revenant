#include "Game/RVBossEncounterVolume.h"
#include "Game/RVGameMode.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "Player/RVPlayerController.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "Kismet/GameplayStatics.h"

ARVBossEncounterVolume::ARVBossEncounterVolume()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ARVBossEncounterVolume::BeginPlay()
{
    Super::BeginPlay();
    OnActorBeginOverlap.AddDynamic(this, &ARVBossEncounterVolume::OnOverlapBegin);
}

void ARVBossEncounterVolume::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
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
    //--- Spawn boss ----------------------------------------------------------

    ensureMsgf(IsValid(BossCharacterClass), TEXT("[ARVBossEncounterVolume] BossCharacterClass is not assigned"));
    if (!IsValid(BossCharacterClass)) { return; }

    const FTransform SpawnTransform = IsValid(BossSpawnPoint)
        ? BossSpawnPoint->GetActorTransform()
        : GetActorTransform();

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    SpawnedBoss = GetWorld()->SpawnActor<ARVSevarogCharacter>(BossCharacterClass, SpawnTransform, Params);

    PauseBossAI();

    //--- Lock input + hide HUD -----------------------------------------------

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (ARVPlayerController* RVPC = Cast<ARVPlayerController>(PC))
    {
        RVPC->LockInputForCutscene();
    }

    if (ARVGameMode* GM = GetWorld()->GetAuthGameMode<ARVGameMode>())
    {
        GM->SetHUDVisible(false);
    }

    //--- Play intro montage --------------------------------------------------

    if (IsValid(SpawnedBoss) && IsValid(BossIntroMontage))
    {
        SpawnedBoss->PlayAnimMontage(BossIntroMontage);
    }

    //--- Start sequence ------------------------------------------------------

    if (!IsValid(CutsceneSequenceActor))
    {
        // No sequence assigned — go straight to gameplay
        if (ARVGameMode* GM = GetWorld()->GetAuthGameMode<ARVGameMode>())
        {
            GM->SetHUDVisible(true);
        }
        if (ARVPlayerController* RVPC = Cast<ARVPlayerController>(PC))
        {
            RVPC->UnlockInputAfterCutscene();
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

    if (IsValid(BossBGM))
    {
        UGameplayStatics::SpawnSound2D(GetWorld(), BossBGM);
    }
}

void ARVBossEncounterVolume::OnCutsceneFinished()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();

    if (ARVGameMode* GM = GetWorld()->GetAuthGameMode<ARVGameMode>())
    {
        GM->SetHUDVisible(true);
    }

    if (ARVPlayerController* RVPC = Cast<ARVPlayerController>(PC))
    {
        RVPC->UnlockInputAfterCutscene();
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
    if (IsValid(AICon->BrainComponent))
    {
        AICon->BrainComponent->StopLogic(TEXT("Cutscene"));
    }
}

void ARVBossEncounterVolume::ResumeBossAI()
{
    if (!IsValid(SpawnedBoss)) { return; }

    AAIController* AICon = Cast<AAIController>(SpawnedBoss->GetController());
    if (!IsValid(AICon)) { return; }

    if (IsValid(AICon->BrainComponent))
    {
        AICon->BrainComponent->RestartLogic();
    }
}