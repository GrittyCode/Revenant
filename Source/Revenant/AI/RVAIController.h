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
    ARVSevarogCharacter* GetBossCharacter() const;

    /** Returns the first player pawn in the world. Used as move and focus target. */
    APawn* GetPlayerPawn() const;


protected:
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UBehaviorTree> BehaviorTreeAsset;
};
