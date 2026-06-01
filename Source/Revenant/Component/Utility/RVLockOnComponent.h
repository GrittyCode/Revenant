// Source/Revenant/Component/Utility/RVLockOnComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVLockOnComponent.generated.h"

class ARVCharacterBase;
class UWidgetComponent;
class UUserWidget;

UCLASS(ClassGroup=(Revenant))
class REVENANT_API URVLockOnComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVLockOnComponent();
    virtual void BeginPlay() override;

    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    void ToggleLockOn();
    void BreakLockOn();

    FORCEINLINE bool IsLockedOn() const { return bIsLockedOn; }
    AActor* GetLockOnTarget() const;

private:
    AActor* TryFindTarget() const;
    void UpdateCamera(float DeltaTime) const;
    void UpdateCharacterRotation(float DeltaTime) const;

    void ShowIndicator();
    void HideIndicator();
    void UpdateIndicatorTransform() const;

    //--- Lock-On Search ------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "RV|LockOn")
    float LockOnRange = 1500.f;

    UPROPERTY(EditDefaultsOnly, Category = "RV|LockOn")
    float LockOnSearchHalfAngle = 60.f;

    UPROPERTY(EditDefaultsOnly, Category = "RV|LockOn")
    float IndicatorHeightOffset = 20.f;

    UPROPERTY(EditDefaultsOnly, Category = "RV|LockOn")
    float AutoBreakRange = 2000.f;

    UPROPERTY(EditDefaultsOnly, Category = "RV|LockOn")
    float CameraInterpSpeed = 5.f;

    UPROPERTY(EditDefaultsOnly, Category = "RV|LockOn")
    float CharacterRotationInterpSpeed = 40.f;

    //--- Indicator -----------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "RV|LockOn|Indicator")
    TSubclassOf<UUserWidget> LockOnIndicatorWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "RV|LockOn|Indicator")
    FVector2D IndicatorDrawSize = FVector2D(64.f, 64.f);

    UPROPERTY()
    TObjectPtr<UWidgetComponent> IndicatorWidgetComp;

    //--- State ---------------------------------------------------------------

    bool bIsLockedOn = false;

    TWeakObjectPtr<AActor> LockOnTarget;

    // ARVCharacterBase is an ACharacter — used for both combat state queries
    // and movement/rotation calls without a second cast.
    UPROPERTY()
    TObjectPtr<ARVCharacterBase> OwnerBase;

    UPROPERTY()
    TObjectPtr<APlayerController> PlayerController;
};