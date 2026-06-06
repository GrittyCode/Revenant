#include "Controller/RVPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Character/Player/RVCharacterPlayer.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "UI/RVHUDWidget.h"
#include "UI/RVBossHPBarWidget.h"
#include "UI/RVGameResultWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

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
        PlayerChar->GetOnHealthChanged().AddUObject(this, &ARVPlayerController::OnPlayerHealthChanged);
        PlayerChar->GetOnStaminaChanged().AddUObject(this, &ARVPlayerController::OnPlayerStaminaChanged);
        PlayerChar->GetOnDeath().AddUObject(this, &ARVPlayerController::OnPlayerDeath);
    }
    else
    {
        UE_LOG(LogRVPlayerController, Warning,
            TEXT("[ARVPlayerController::BeginPlay] GetPawn() is null — attribute delegates not bound."));
    }
}

void ARVPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
    if (!IsValid(EIC) || !IsValid(SkipCutsceneAction)) { return; }

    EIC->BindAction(SkipCutsceneAction, ETriggerEvent::Started, this, &ARVPlayerController::OnSkipCutsceneInput);
}

//--- Input helpers -----------------------------------------------------------

UEnhancedInputLocalPlayerSubsystem* ARVPlayerController::GetInputSubsystem() const
{
    return GetLocalPlayer()
        ? GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()
        : nullptr;
}

void ARVPlayerController::LockInputForCutscene()
{
    ARVCharacterPlayer* PlayerChar = Cast<ARVCharacterPlayer>(GetPawn());
    if (!ensureMsgf(IsValid(PlayerChar),
        TEXT("[ARVPlayerController::LockInputForCutscene] Pawn is not ARVCharacterPlayer"))) { return; }

    UEnhancedInputLocalPlayerSubsystem* Subsystem = GetInputSubsystem();
    if (!ensureMsgf(IsValid(Subsystem),
        TEXT("[ARVPlayerController::LockInputForCutscene] EnhancedInputLocalPlayerSubsystem missing"))) { return; }
    if (!ensureMsgf(IsValid(CutsceneMappingContext),
        TEXT("[ARVPlayerController::LockInputForCutscene] CutsceneMappingContext not assigned — assign IMC_Cutscene in BP_RVPlayerController"))) { return; }

    // Remove all player actions; cache for restoration.
    UInputMappingContext* PlayerIMC = PlayerChar->GetDefaultMappingContext();
    if (IsValid(PlayerIMC))
    {
        Subsystem->RemoveMappingContext(PlayerIMC);
        CachedPlayerMappingContext = PlayerIMC;
    }

    // Add cutscene context — only IA_SkipCutscene is mapped here.
    Subsystem->AddMappingContext(CutsceneMappingContext, 0);
}

void ARVPlayerController::UnlockInputAfterCutscene()
{
    UEnhancedInputLocalPlayerSubsystem* Subsystem = GetInputSubsystem();
    if (!ensureMsgf(IsValid(Subsystem),
        TEXT("[ARVPlayerController::UnlockInputAfterCutscene] EnhancedInputLocalPlayerSubsystem missing"))) { return; }

    if (IsValid(CutsceneMappingContext))
    {
        Subsystem->RemoveMappingContext(CutsceneMappingContext);
    }

    if (CachedPlayerMappingContext.IsValid())
    {
        Subsystem->AddMappingContext(CachedPlayerMappingContext.Get(), 0);
        CachedPlayerMappingContext.Reset();
    }

    FlushPressedKeys();
}

void ARVPlayerController::OnSkipCutsceneInput()
{
    OnCutsceneSkipRequested.Broadcast();
}

//--- HUD visibility ----------------------------------------------------------

void ARVPlayerController::SetHUDVisible(bool bVisible)
{
    if (!IsValid(HUDWidget)) { return; }
    HUDWidget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

//--- Game input restore ------------------------------------------------------

void ARVPlayerController::RestoreGameInput()
{
    SetInputMode(FInputModeGameOnly());
    bShowMouseCursor = false;
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

    InBoss->OnBossDefeated.AddUObject(this, &ARVPlayerController::OnBossDefeated);
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