#include "UI/RVHUDWidget.h"
#include "Components/ProgressBar.h"

void URVHUDWidget::SetHPPercent(float InPercent)
{
	if (HPBar) { HPBar->SetPercent(InPercent); }
}

void URVHUDWidget::SetStaminaPercent(float InPercent)
{
	if (StaminaBar) { StaminaBar->SetPercent(InPercent); }
}