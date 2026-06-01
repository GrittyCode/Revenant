#include "Controller/RVPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Character/Player/RVCharacterPlayer.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "UI/RVHUDWidget.h"
#include "UI/RVBossHPBarWidget.h"
#include "UI/RVGameResultWidget.h"

DEFINE_LOG_CATEGORY(LogRVPlayerController);

void ARVPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (IsValid(HUDWidgetClass))
    {
        HUDWidget = CreateWidget<URVHUDWidget>(this, HUDWidgetClass);
        HUDWidget->AddToViewport();
    }

    if (IsValid(BossHPBarWidgetClass))
    {
        BossHPBarWidget = CreateWidget<URVBossHPBarWidget>(this, BossHPBarWidgetClass);
    }

    if (IsValid(GameResultWidgetClass))
    {
        GameResultWidget = CreateWidget<URVGameResultWidget>(this, GameResultWidgetClass);
    }

    if (ARVCharacterPlayer* PlayerChar = Cast<ARVCharacterPlayer>(GetPawn()))
    {
        PlayerChar->GetOnHealthChanged().AddDynamic(this, &ARVPlayerController::OnPlayerHealthChanged);
        PlayerChar->GetOnStaminaChanged().AddDynamic(this, &ARVPlayerController::OnPlayerStaminaChanged);
        PlayerChar->GetOnDeath().AddDynamic(this, &ARVPlayerController::OnPlayerDeath);
    }
    else
    {
        UE_LOG(LogRVPlayerController, Warning,
            TEXT("[ARVPlayerController::BeginPlay] GetPawn() is null — attribute delegates not bound."));
    }
}

//--- Input helpers -----------------------------------------------------------

void ARVPlayerController::RestoreGameInput()
{
    SetInputMode(FInputModeGameOnly());
    bShowMouseCursor = false;
}

void ARVPlayerController::LockInputForCutscene()
{
    SetIgnoreMoveInput(true);
    SetIgnoreLookInput(true);
    SetInputMode(FInputModeUIOnly());
}

void ARVPlayerController::UnlockInputAfterCutscene()
{
    FlushPressedKeys();
    ResetIgnoreMoveInput();
    ResetIgnoreLookInput();
    SetInputMode(FInputModeGameOnly());
    bShowMouseCursor = false;
}

//--- HUD visibility ----------------------------------------------------------

void ARVPlayerController::SetHUDVisible(bool bVisible)
{
    if (!IsValid(HUDWidget)) { return; }
    HUDWidget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

//--- Boss lifecycle ----------------------------------------------------------

void ARVPlayerController::OnBossSpawned(ARVSevarogCharacter* InBoss)
{
    if (!IsValid(InBoss)) { return; }

    BossRef = InBoss;

    if (IsValid(BossHPBarWidget))
    {
        BossHPBarWidget->SetBoss(InBoss);
        BossHPBarWidget->AddToViewport();
    }

    InBoss->OnBossDefeated.AddDynamic(this, &ARVPlayerController::OnBossDefeated);
}

void ARVPlayerController::OnBossDefeated()
{
    if (IsValid(BossHPBarWidget) && BossHPBarWidget->IsInViewport())
    {
        BossHPBarWidget->RemoveFromParent();
    }
    ShowGameResult(true);
}

//--- Game result -------------------------------------------------------------

void ARVPlayerController::ShowGameResult(bool bVictory)
{
    if (!IsValid(GameResultWidget)) { return; }

    GameResultWidget->SetResult(bVictory);
    GameResultWidget->AddToViewport();

    SetInputMode(FInputModeUIOnly());
    bShowMouseCursor = true;
}

//--- Player attribute handlers -----------------------------------------------

void ARVPlayerController::OnPlayerHealthChanged(float NewHealthRatio)
{
    if (!IsValid(HUDWidget)) { return; }
    HUDWidget->SetHPPercent(NewHealthRatio);
}

void ARVPlayerController::OnPlayerStaminaChanged(float NewStaminaRatio)
{
    if (!IsValid(HUDWidget)) { return; }
    HUDWidget->SetStaminaPercent(NewStaminaRatio);
}

void ARVPlayerController::OnPlayerDeath()
{
    if (IsValid(BossHPBarWidget) && BossHPBarWidget->IsInViewport())
    {
        BossHPBarWidget->RemoveFromParent();
    }

    ShowGameResult(false);
}