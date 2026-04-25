// Source/Revenant/Character/Base/RVCharacterBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/RVCombatInterface.h"
#include "RVCharacterBase.generated.h"

class URVCharacterDataAsset;
class URVAttributeComponent;
class URVComboComponent;
class URVEquipmentComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogRVCharacterBase, Log, All);

UCLASS()
class REVENANT_API ARVCharacterBase : public ACharacter, public IRVCombatInterface
{
	GENERATED_BODY()

public:
	ARVCharacterBase();

	// Combat Section
public:
	virtual void ActivateHitCheck_Implementation() override;
protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
	TObjectPtr<URVAttributeComponent> AttributeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
	TObjectPtr<URVComboComponent> ComboComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
	TObjectPtr<URVEquipmentComponent> EquipmentComponent;

	// Assign the matching DataAsset in each character's Blueprint defaults
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Data")
	TObjectPtr<URVCharacterDataAsset> CharacterData;

private:
	void InitializeComponents();
};