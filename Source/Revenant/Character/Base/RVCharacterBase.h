// Source/Revenant/Character/Base/RVCharacterBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RVCharacterBase.generated.h"

// Data
class URVCharacterDataAsset;


// Component
class URVAttributeComponent;
class URVComboComponent;


DECLARE_LOG_CATEGORY_EXTERN(LogRVCharacterBase, Log, All);

UCLASS()
class REVENANT_API ARVCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ARVCharacterBase();

	// IRVDamageable — to be added in W2 when the interface is introduced
	// virtual bool ApplyDamage(float InDamageAmount, AActor* InInstigator);

protected:
	virtual void BeginPlay() override;
	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
	TObjectPtr<URVAttributeComponent> AttributeComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RV|Components")
	TObjectPtr<URVComboComponent> ComboComponent;

	// Assign the matching DataAsset in each character's Blueprint defaults
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Data")
	TObjectPtr<URVCharacterDataAsset> CharacterData;
	
	

private:
	void InitializeComponents();
};