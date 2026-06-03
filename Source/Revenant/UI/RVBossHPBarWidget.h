#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RVBossHPBarWidget.generated.h"

class UProgressBar;
class UTextBlock;
class ARVSevarogCharacter;

UCLASS(Abstract)
class REVENANT_API URVBossHPBarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetBoss(ARVSevarogCharacter* InBoss);

protected:
    virtual void NativeDestruct() override;

private:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> BossNameText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> HPBar;

    // Fills as poise is consumed (0 = full poise, 1 = poise depleted → Groggy).
    // Resets to 0 when Groggy ends.
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> PoiseBar;

    TWeakObjectPtr<ARVSevarogCharacter> BossRef;

    //--- Delegate handlers ---------------------------------------------------
	
    void OnBossHealthChanged(float NewHealthRatio);
	void OnBossPoiseChangedHandler(float NewPoiseRatio);
    void OnGroggyStarted();
    void OnGroggyEnded();
};
