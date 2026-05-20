#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "RVAIController.generated.h"

class UBehaviorTree;
class ARVSevarogCharacter;

UCLASS()
class REVENANT_API ARVAIController : public AAIController
{
	GENERATED_BODY()

public:
	ARVAIController();

	/** Returns the possessed ARVSevarogCharacter. Null if not yet possessed. */
	UFUNCTION(BlueprintCallable, Category = "RV|AI")
	ARVSevarogCharacter* GetBossCharacter() const;

	/** Returns the first player pawn in the world. Used as move and focus target. */
	UFUNCTION(BlueprintCallable, Category = "RV|AI")
	APawn* GetPlayerPawn() const;

	/** Rotates the boss to face the player via controller focus. */
	UFUNCTION(BlueprintCallable, Category = "RV|AI")
	void SetFocusToPlayer();

	/** Clears focus so the boss stops rotating toward a specific actor. */
	UFUNCTION(BlueprintCallable, Category = "RV|AI")
	void ClearBossFocus();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

protected:
	// RunBehaviorTree handles blackboard initialization internally via the BT asset.
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;
};