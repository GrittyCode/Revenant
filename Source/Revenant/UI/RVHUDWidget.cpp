#include "UI/RVHUDWidget.h"
#include "Components/ProgressBar.h"


void URVHUDWidget::SetHPPercent(float InPercent)
{
    HPBar->SetPercent(InPercent);
}

void URVHUDWidget::SetStaminaPercent(float InPercent)
{
    StaminaBar->SetPercent(InPercent);
}
