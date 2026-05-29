#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVLockOnComponent.generated.h"

class ARVCharacterBase;
class ACharacter;
class APlayerController;
class UWidgetComponent;
class UUserWidget;

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVLockOnComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URVLockOnComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    void ToggleLockOn();
    void BreakLockOn();

    bool    IsLockedOn()      const { return bIsLockedOn; }
    AActor* GetLockOnTarget() const;

protected:
    virtual void BeginPlay() override;

private:
    AActor* TryFindTarget() const;
    void    UpdateCamera(float DeltaTime) const;
    void    UpdateCharacterRotation(float DeltaTime) const;

    void    ShowIndicator();
    void    HideIndicator();
    void    UpdateIndicatorTransform() const;

    // --- Lock-On Search ---

    UPROPERTY(EditDefaultsOnly, Category = "RV|LockOn")
    float LockOnRange = 1500.f;

    UPROPERTY(EditDefaultsOnly, Category = "RV|LockOn")
    float LockOnSearchHalfAngle = 60.f;

    UPROPERTY(EditDefaultsOnly, Category = "RV|LockOn")
    float AutoBreakRange = 2000.f;

    UPROPERTY(EditDefaultsOnly, Category = "RV|LockOn")
    float CameraInterpSpeed = 5.f;

    UPROPERTY(EditDefaultsOnly, Category = "RV|LockOn")
    float CharacterRotationInterpSpeed = 40.f;

    // --- Indicator ---

    UPROPERTY(EditDefaultsOnly, Category = "RV|LockOn|Indicator")
    TSubclassOf<UUserWidget> LockOnIndicatorWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "RV|LockOn|Indicator")
    FVector2D IndicatorDrawSize = FVector2D(64.f, 64.f);

    UPROPERTY(EditDefaultsOnly, Category = "RV|LockOn|Indicator")
    float IndicatorZOffset = 80.f;

    // Runtime — created in BeginPlay, lives on the owner actor.
    UPROPERTY()
    TObjectPtr<UWidgetComponent> IndicatorWidgetComp;

    // --- State ---

    bool bIsLockedOn = false;

    TWeakObjectPtr<AActor> LockOnTarget;

    // OwnerBase for combat state queries (HasCombatState).
    // OwnerCharacter for movement component and transform access.
    // Both point to the same actor — held separately for type clarity.
    UPROPERTY()
    TObjectPtr<ARVCharacterBase> OwnerBase;

    UPROPERTY()
    TObjectPtr<ACharacter> OwnerCharacter;

    // Cached in BeginPlay via GetOwner()->GetController().
    UPROPERTY()
    TObjectPtr<APlayerController> PlayerController;
};