#include "Game/RVBossEncounterVolume.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "Camera/PlayerCameraManager.h"
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
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (IsValid(PC) && IsValid(PC->PlayerCameraManager))
    {
        PC->PlayerCameraManager->StartCameraFade(0.f, 1.f, FadeOutDuration, FadeColor, false, true);
    }

    GetWorldTimerManager().SetTimer(
        SpawnTimerHandle, this, &ARVBossEncounterVolume::OnFadeOutComplete, FadeOutDuration, false);
}

void ARVBossEncounterVolume::OnFadeOutComplete()
{
    ensureMsgf(IsValid(BossCharacterClass), TEXT("[ARVBossEncounterVolume] BossCharacterClass is not assigned"));
    if (!IsValid(BossCharacterClass)) { return; }

    const FTransform SpawnTransform = IsValid(BossSpawnPoint)
        ? BossSpawnPoint->GetActorTransform()
        : GetActorTransform();

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    SpawnedBoss = GetWorld()->SpawnActor<ARVSevarogCharacter>(BossCharacterClass, SpawnTransform, Params);

    if (IsValid(BossBGM))
    {
        UGameplayStatics::SpawnSound2D(GetWorld(), BossBGM);
    }

    GetWorldTimerManager().SetTimer(
        FadeInTimerHandle, this, &ARVBossEncounterVolume::OnFadeInStart, SpawnDelay, false);
}

void ARVBossEncounterVolume::OnFadeInStart()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (IsValid(PC) && IsValid(PC->PlayerCameraManager))
    {
        PC->PlayerCameraManager->StartCameraFade(1.f, 0.f, FadeInDuration, FadeColor, false, false);
    }

    OnBossSpawned.Broadcast(SpawnedBoss);
}