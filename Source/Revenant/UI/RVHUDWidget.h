#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RVHUDWidget.generated.h"

class UProgressBar;
class UImage;
class UTexture2D;

UCLASS(Abstract)
class REVENANT_API URVHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetHPPercent(float InPercent);
	void SetStaminaPercent(float InPercent);

	// Called once after widget creation.
	// Sets the icon brush for each weapon slot.
	// Null textures are silently ignored — icon stays blank.
	void InitWeaponSlots(UTexture2D* InIconA, UTexture2D* InIconB);

	// Highlights the active slot; dims the inactive one.
	// bIsSlotA = true  → SlotA full opacity, SlotB dimmed.
	// bIsSlotA = false → SlotB full opacity, SlotA dimmed.
	void SetActiveWeaponSlot(bool bIsSlotA);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HPBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaBar;
};