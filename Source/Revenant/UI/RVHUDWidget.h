#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RVHUDWidget.generated.h"

class UProgressBar;

// [버그-2] InitWeaponSlots / SetActiveWeaponSlot — 선언만 있고 구현이 없던 데드 메서드 제거.
// 관련 UImage, UTexture2D forward declaration과 include도 함께 제거.
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
