#include "UI/RVGameResultWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void URVGameResultWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RetryButton->OnClicked.AddDynamic(this, &URVGameResultWidget::OnRetryClicked);
}

void URVGameResultWidget::NativeDestruct()
{
	RetryButton->OnClicked.RemoveDynamic(this, &URVGameResultWidget::OnRetryClicked);
	Super::NativeDestruct();
}

void URVGameResultWidget::SetResult(bool bVictory)
{
	ResultText->SetText(bVictory
		? FText::FromString(TEXT("Victory"))
		: FText::FromString(TEXT("Defeat")));
}

void URVGameResultWidget::OnRetryClicked()
{
	const FString LevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	UGameplayStatics::OpenLevel(this, FName(*LevelName));
}