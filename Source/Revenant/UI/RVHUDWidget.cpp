#include "UI/RVHUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

// Opacity applied to the inactive weapon slot icon.
static constexpr float InactiveSlotOpacity = 0.35f;

// ----------------------------------------------------------------------------

void URVHUDWidget::SetHPPercent(float InPercent)
{
	if (HPBar) { HPBar->SetPercent(InPercent); }
}

void URVHUDWidget::SetStaminaPercent(float InPercent)
{
	if (StaminaBar) { StaminaBar->SetPercent(InPercent); }
}
