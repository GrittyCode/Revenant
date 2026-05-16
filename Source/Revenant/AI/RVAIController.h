// Source/Revenant/AI/RVAIController.h
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "RVAIController.generated.h"

class UStateTreeAIComponent;
class ARVBossCharacter;

/**
 * ARVAIController
 *
 * Boss AI controller. Owns UStateTreeAIComponent — behavior logic is authored
 * in a Blueprint StateTree asset (ST_BossAI). C++ provides the frame and
 * helper accessors used by StateTree tasks.
 *
 * StateTree task authors call:
 *   GetBossCharacter()      — to read boss state (IsAttacking, IsGroggy, Phase)
 *   GetPlayerPawn()         — to get the player as a move/focus target
 *   SetFocusToPlayer()      — to make the boss face the player
 */
UCLASS()
class REVENANT_API ARVAIController : public AAIController
{
	GENERATED_BODY()

public:
	ARVAIController();

	/** Returns the possessed ARVBossCharacter. Null if not yet possessed. */
	UFUNCTION(BlueprintCallable, Category = "RV|AI")
	ARVBossCharacter* GetBossCharacter() const;

	/** Returns the first player pawn in the world. Used as move and focus target. */
	UFUNCTION(BlueprintCallable, Category = "RV|AI")
	APawn* GetPlayerPawn() const;

	/** Sets the controller focus to the player pawn so the boss rotates toward them. */
	UFUNCTION(BlueprintCallable, Category = "RV|AI")
	void SetFocusToPlayer();

	/** Clears focus so the boss stops rotating toward a specific actor. */
	UFUNCTION(BlueprintCallable, Category = "RV|AI")
	void ClearBossFocus();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "RV|AI")
	TObjectPtr<UStateTreeAIComponent> StateTreeAIComponent;
};