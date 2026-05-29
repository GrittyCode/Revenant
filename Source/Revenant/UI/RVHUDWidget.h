#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RVHUDWidget.generated.h"

class UProgressBar;

UCLASS(Abstract)
class REVENANT_API URVHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetHPPercent(float InPercent);
    void SetStaminaPercent(float InPercent);

private:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> HPBar;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> StaminaBar;
};
