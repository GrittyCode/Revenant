#include "UI/RVHUDWidget.h"
#include "Components/ProgressBar.h"

// [버그-2] 구현 없는 InitWeaponSlots/SetActiveWeaponSlot 제거.
// Components/Image.h, Engine/Texture2D.h, InactiveSlotOpacity 상수도 함께 제거.

void URVHUDWidget::SetHPPercent(float InPercent)
{
    // [IsValid-7] UCLASS(Abstract)+BindWidget 계약: WBP에 HPBar 없으면 컴파일 에러.
    // 정상 빌드에서 항상 non-null — IsValid 불필요.
    HPBar->SetPercent(InPercent);
}

void URVHUDWidget::SetStaminaPercent(float InPercent)
{
    // [IsValid-7] 동일.
    StaminaBar->SetPercent(InPercent);
}
