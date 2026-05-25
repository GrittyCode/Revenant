#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/RVHitCheckTarget.h"
#include "Interface/RVDamageable.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "RVCharacterBase.generated.h"

class URVHitReactionComponent;
class URVCharacterDataAsset;
class URVHitReactionAnimDataAsset;
class UMeshComponent;

UCLASS()
class REVENANT_API ARVCharacterBase : public ACharacter, public IRVHitCheckTarget, public IRVDamageable
{
    GENERATED_BODY()

public:
    ARVCharacterBase();

    virtual void ActivateHitCheck() override;
    virtual bool ApplyDamage(const FRVHitInfo& InHitInfo) override;

    UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
    float GetHealthRatio() const;

    UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
    float GetStaminaRatio() const;

    //--- Attribute event facades ---------------------------------------------

    FRVOnHealthChanged&  GetOnHealthChanged();
    FRVOnStaminaChanged& GetOnStaminaChanged();
    FRVOnDeath&          GetOnDeath();
    FRVOnPoiseDepleted&  GetOnPoiseDepleted();
    FRVOnPoiseChanged&   GetOnPoiseChanged();

    //--- Combat state facades (AnimNotify) -----------------------------------

    void OpenAttackHitWindow();
    void CloseAttackHitWindow();

    //--- State query facades (AnimInstance) ----------------------------------

    bool  IsInCombatState(ERVCombatState InState) const;
    float GetStaggerDirection() const;

protected:
    virtual void BeginPlay() override;
    virtual void Falling() override;
    virtual void Landed(const FHitResult& Hit) override;

    virtual URVHitReactionAnimDataAsset* GetHitReactionAnimData() const { return nullptr; }

    // Called from BeginPlay before HitReactionComponent is initialized.
    // Subclass initializes AttributeComponent and any character-specific stats here.
    virtual void InitStats() {}

    UFUNCTION()
    virtual void OnDeath();

    virtual UMeshComponent* GetWeaponTraceMesh() const { return GetMesh(); }

    //--- Spatial helpers -----------------------------------------------------

    FVector GetForwardLocation(float InOffset = 1.f) const;
    FVector GetGroundOrigin() const;

    //--- Components ----------------------------------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVAttributeComponent> AttributeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVCombatStateComponent> CombatStateComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVHitReactionComponent> HitReactionComponent;

    //--- Data ----------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Data")
    TObjectPtr<URVCharacterDataAsset> CharacterData;

    //--- Movement ------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "RV|Movement")
    FRotator AirRotationRate = FRotator(0.f, 0.f, 0.f);

private:
    FRotator OriginalRotationRate;
};