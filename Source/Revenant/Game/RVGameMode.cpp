#include "Game/RVGameMode.h"
#include "Game/RVBossEncounterVolume.h"
#include "UI/RVHUDWidget.h"
#include "UI/RVBossHPBarWidget.h"
#include "UI/RVGameResultWidget.h"
#include "Character/Base/RVCharacterBase.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void ARVGameMode::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    ensureMsgf(IsValid(PC), TEXT("[ARVGameMode] No PlayerController at BeginPlay"));
    if (!IsValid(PC)) { return; }

    //--- Create widgets ------------------------------------------------------

    if (IsValid(HUDWidgetClass))
    {
        HUDWidget = CreateWidget<URVHUDWidget>(PC, HUDWidgetClass);
        HUDWidget->AddToViewport();
    }

    if (IsValid(BossHPBarWidgetClass))
    {
        BossHPBarWidget = CreateWidget<URVBossHPBarWidget>(PC, BossHPBarWidgetClass);
    }

    if (IsValid(GameResultWidgetClass))
    {
        GameResultWidget = CreateWidget<URVGameResultWidget>(PC, GameResultWidgetClass);
    }

    //--- Bind player delegates via CharacterBase facade ----------------------

    if (ARVCharacterBase* PlayerChar = Cast<ARVCharacterBase>(PC->GetPawn()))
    {
        PlayerCharRef = PlayerChar;
        PlayerChar->GetOnHealthChanged().AddDynamic(this, &ARVGameMode::OnPlayerHealthChanged);
        PlayerChar->GetOnStaminaChanged().AddDynamic(this, &ARVGameMode::OnPlayerStaminaChanged);
        PlayerChar->GetOnDeath().AddDynamic(this, &ARVGameMode::OnPlayerDeath);
    }
	
	
    //--- Subscribe to BossEncounterVolume ------------------------------------

    AActor* VolumeActor = UGameplayStatics::GetActorOfClass(GetWorld(), ARVBossEncounterVolume::StaticClass());
    if (ARVBossEncounterVolume* Volume = Cast<ARVBossEncounterVolume>(VolumeActor))
    {
        Volume->OnBossSpawned.AddUObject(this, &ARVGameMode::OnBossSpawnedHandler);
    }
}

void ARVGameMode::OnBossSpawnedHandler(ARVSevarogCharacter* InBoss)
{
    if (!IsValid(InBoss)) { return; }

    if (IsValid(BossHPBarWidget))
    {
        BossHPBarWidget->SetBoss(InBoss);
        BossHPBarWidget->AddToViewport();
    }

    InBoss->OnBossDefeated.AddDynamic(this, &ARVGameMode::OnBossDefeated);
}

void ARVGameMode::OnBossDefeated()
{
    if (IsValid(BossHPBarWidget) && BossHPBarWidget->IsInViewport())
    {
        BossHPBarWidget->RemoveFromParent();
    }
    ShowGameResult(true);
}

void ARVGameMode::ShowGameResult(bool bVictory)
{
    if (!IsValid(GameResultWidget)) { return; }

    GameResultWidget->SetResult(bVictory);
    GameResultWidget->AddToViewport();

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (IsValid(PC))
    {
        PC->SetInputMode(FInputModeUIOnly());
        PC->bShowMouseCursor = true;
    }
}

void ARVGameMode::OnPlayerHealthChanged(float NewHealth, float InDelta)
{
    if (!IsValid(HUDWidget) || !PlayerCharRef.IsValid()) { return; }
    HUDWidget->SetHPPercent(PlayerCharRef->GetHealthRatio());
}

void ARVGameMode::OnPlayerStaminaChanged(float NewStamina, float InDelta)
{
	if (!IsValid(HUDWidget) || !PlayerCharRef.IsValid()) { return; }
	HUDWidget->SetStaminaPercent(PlayerCharRef->GetStaminaRatio());
}

void ARVGameMode::OnPlayerDeath()
{
    if (IsValid(BossHPBarWidget) && BossHPBarWidget->IsInViewport())
    {
        BossHPBarWidget->RemoveFromParent();
    }
    ShowGameResult(false);
}