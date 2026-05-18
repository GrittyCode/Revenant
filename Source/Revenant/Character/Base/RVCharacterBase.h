#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/RVCombatInterface.h"
#include "Interface/RVDamageable.h"
#include "RVCharacterBase.generated.h"

class URVAttributeComponent;
class URVCombatStateComponent;
class URVHitReactionComponent;
class URVCharacterDataAsset;
class URVHitReactionAnimDataAsset;
class UMeshComponent;

UCLASS()
class REVENANT_API ARVCharacterBase : public ACharacter, public IRVCombatInterface, public IRVDamageable
{
    GENERATED_BODY()

public:
    ARVCharacterBase();

    virtual void ActivateHitCheck() override;
    virtual bool ApplyDamage(const FRVHitInfo& InHitInfo) override;
	
	UFUNCTION(BlueprintCallable, Category = "RV|Attribute")
	float GetHealthRatio() const;

protected:
    virtual void BeginPlay() override;
    virtual void Falling() override;
    virtual void Landed(const FHitResult& Hit) override;


    virtual URVHitReactionAnimDataAsset* GetHitReactionAnimData() const { return nullptr; }

    /**
     * Player: weapon StaticMeshComponent (set up by URVEquipmentComponent).
     * Boss:   character SkeletalMeshComponent (sockets on Sevarog skeleton).
     */
    virtual UMeshComponent* GetWeaponTraceMesh() const { return GetMesh(); }

    // --- Components ----------------------------------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVAttributeComponent> AttributeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVCombatStateComponent> CombatStateComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
    TObjectPtr<URVHitReactionComponent> HitReactionComponent;

    // --- Data ----------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Data")
    TObjectPtr<URVCharacterDataAsset> CharacterData;

    // --- Movement ------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "RV|Movement")
    FRotator AirRotationRate = FRotator(0.f, 0.f, 0.f);

private:
    FRotator OriginalRotationRate;
};