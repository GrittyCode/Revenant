#include "Game/RVBossEncounterVolume.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "Player/RVPlayerController.h"
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
    //--- Spawn boss ----------------------------------------------------------

    ensureMsgf(IsValid(BossCharacterClass),
        TEXT("[ARVBossEncounterVolume] BossCharacterClass is not assigned"));
    if (!IsValid(BossCharacterClass)) { return; }

    const FTransform SpawnTransform = IsValid(BossSpawnPoint)
        ? BossSpawnPoint->GetActorTransform()
        : GetActorTransform();

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    SpawnedBoss = GetWorld()->SpawnActor<ARVSevarogCharacter>(BossCharacterClass, SpawnTransform, Params);
    SpawnedBoss->SetActorHiddenInGame(true);

    PauseBossAI();

    //--- Lock input + hide HUD (all routed through ARVPlayerController) ------

    if (ARVPlayerController* RVPC = Cast<ARVPlayerController>(GetWorld()->GetFirstPlayerController()))
    {
        RVPC->LockInputForCutscene();
        RVPC->SetHUDVisible(false);
    }

    StartCutscene();
}

void ARVBossEncounterVolume::StartCutscene()
{
    ARVPlayerController* RVPC = Cast<ARVPlayerController>(GetWorld()->GetFirstPlayerController());

    if (!IsValid(CutsceneSequenceActor))
    {
        // No sequence — go straight to gameplay.
        if (IsValid(RVPC))
        {
            RVPC->SetHUDVisible(true);
            RVPC->UnlockInputAfterCutscene();
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

    //--- Play intro montage --------------------------------------------------

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
    // [버그-1] CutsceneBGM 미할당 레벨에서 CutsceneBGMAudioComponent가 null → 크래시 방지.
    if (IsValid(CutsceneBGMAudioComponent))
    {
        CutsceneBGMAudioComponent->Stop();
    }

    if (ARVPlayerController* RVPC = Cast<ARVPlayerController>(GetWorld()->GetFirstPlayerController()))
    {
        RVPC->SetHUDVisible(true);
        RVPC->UnlockInputAfterCutscene();
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
