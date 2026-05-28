#include "UI/RVGameResultWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void URVGameResultWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // [IsValid-10] BindWidget 계약 — RetryButton은 항상 non-null.
    RetryButton->OnClicked.AddDynamic(this, &URVGameResultWidget::OnRetryClicked);
}

void URVGameResultWidget::NativeDestruct()
{
    // [IsValid-10] 동일.
    RetryButton->OnClicked.RemoveDynamic(this, &URVGameResultWidget::OnRetryClicked);

    Super::NativeDestruct();
}

void URVGameResultWidget::SetResult(bool bVictory)
{
    // [IsValid-10] BindWidget — ResultText 항상 non-null.
    ResultText->SetText(bVictory
        ? FText::FromString(TEXT("Victory"))
        : FText::FromString(TEXT("Defeat")));
}

void URVGameResultWidget::OnRetryClicked()
{
    UGameplayStatics::OpenLevel(GetWorld(), FName(*GetWorld()->GetName()));
}
