#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RVGameResultWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS(Abstract)
class REVENANT_API URVGameResultWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetResult(bool bVictory);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ResultText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RetryButton;

	UFUNCTION()
	void OnRetryClicked();
};