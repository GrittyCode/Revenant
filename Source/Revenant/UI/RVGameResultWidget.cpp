#include "UI/RVGameResultWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void URVGameResultWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (RetryButton)
	{
		RetryButton->OnClicked.AddDynamic(this, &URVGameResultWidget::OnRetryClicked);
	}
}

void URVGameResultWidget::NativeDestruct()
{
	if (RetryButton)
	{
		RetryButton->OnClicked.RemoveDynamic(this, &URVGameResultWidget::OnRetryClicked);
	}

	Super::NativeDestruct();
}

void URVGameResultWidget::SetResult(bool bVictory)
{
	if (ResultText)
	{
		ResultText->SetText(bVictory
			? FText::FromString(TEXT("Victory"))
			: FText::FromString(TEXT("Defeat")));
	}
}

void URVGameResultWidget::OnRetryClicked()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName(*GetWorld()->GetName()));
}