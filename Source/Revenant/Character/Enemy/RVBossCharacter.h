#pragma once

#include "CoreMinimal.h"
#include "Character/Base/RVCharacterBase.h"
#include "RVBossCharacter.generated.h"

class URVBossDataAsset;

UENUM(BlueprintType)
enum class ERVBossPhase : uint8
{
    Phase1 UMETA(DisplayName = "Phase 1"),
    Phase2 UMETA(DisplayName = "Phase 2"),
    Phase3 UMETA(DisplayName = "Phase 3"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRVOnBossPhaseChanged, ERVBossPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRVOnBossGroggyStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRVOnBossGroggyEnded);

UCLASS()
class REVENANT_API ARVBossCharacter : public ARVCharacterBase
{
    GENERATED_BODY()

public:
    ARVBossCharacter();

    // --- StateTree task interface --------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "RV|Boss")
    void ExecuteBossAttack(UAnimMontage* InAttackMontage);

    UFUNCTION(BlueprintCallable, Category = "RV|Boss")
    void StartGroggy();

    UFUNCTION(BlueprintCallable, Category = "RV|Boss")
    void EndGroggy();

    UFUNCTION(BlueprintCallable, Category = "RV|Boss")
    ERVBossPhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintCallable, Category = "RV|Boss")
    bool IsGroggy() const { return bIsGroggy; }

    UFUNCTION(BlueprintCallable, Category = "RV|Boss")
    bool IsAttacking() const;

    // --- Delegates -----------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "RV|Boss")
    FRVOnBossPhaseChanged OnBossPhaseChanged;

    UPROPERTY(BlueprintAssignable, Category = "RV|Boss")
    FRVOnBossGroggyStarted OnBossGroggyStarted;

    UPROPERTY(BlueprintAssignable, Category = "RV|Boss")
    FRVOnBossGroggyEnded OnBossGroggyEnded;

protected:
    virtual void BeginPlay() override;

    // Sevarog's SkeletalMeshComponent owns WeaponRoot / WeaponTip sockets.
    // Base GetWeaponTraceMesh() already returns GetMesh() — no override needed.
    virtual URVCombatDataAsset* GetCombatData() const override;

    UPROPERTY(EditDefaultsOnly, Category = "RV|Boss")
    TObjectPtr<URVBossDataAsset> BossData;

private:
    ERVBossPhase CurrentPhase = ERVBossPhase::Phase1;
    bool bIsGroggy = false;
    int32 CurrentPoiseDepletionCount = 0;

    FTimerHandle GroggyTimerHandle;

    void SetBossPhase(ERVBossPhase InNewPhase);

    UFUNCTION()
    void CheckPhaseTransition(float InNewHealth, float InDelta);

    UFUNCTION()
    void OnPoiseDepleted();

	UFUNCTION(BlueprintCallable, Category = "RV|Boss")
	void ExecutePhaseAttack();

    void OnAttackMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);
	
};