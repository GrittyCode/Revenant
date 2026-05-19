#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RVPlayerCombatAnimDataAsset.generated.h"

class UAnimMontage;

UENUM(BlueprintType)
enum class ERVDodgeDirection : uint8
{
	Forward    UMETA(DisplayName = "Forward"),
	Left       UMETA(DisplayName = "Left"),
	Right      UMETA(DisplayName = "Right"),
	BackLeft   UMETA(DisplayName = "Back Left"),
	BackRight  UMETA(DisplayName = "Back Right"),
};

UCLASS(BlueprintType)
class REVENANT_API URVPlayerCombatAnimDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	//--- Combo ---------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Combo")
	TArray<TObjectPtr<UAnimMontage>> ComboMontages;

	UAnimMontage* GetComboMontage(int32 InIndex) const;
	int32 GetMaxComboCount() const { return ComboMontages.Num(); }
	int32 FindComboMontageIndex(const UAnimMontage* InMontage) const;

	//--- Heavy Attack --------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|HeavyAttack")
	TObjectPtr<UAnimMontage> HeavyChargeMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|HeavyAttack")
	TObjectPtr<UAnimMontage> HeavyAttackMontage;

	// Played when charge completes without manual release (max charge).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|HeavyAttack")
	TObjectPtr<UAnimMontage> MaxHeavyAttackMontage;

	//--- Dodge ---------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Dodge")
	TObjectPtr<UAnimMontage> DodgeMontage;

	// Lock-on dodge montages keyed by dodge direction.
	// Falls back to DodgeMontage if a direction entry is missing or unassigned.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Dodge|LockOn")
	TMap<ERVDodgeDirection, TObjectPtr<UAnimMontage>> LockOnDodgeMontages;

	//--- Guard ---------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Guard")
	TObjectPtr<UAnimMontage> GuardHitMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RV|Guard")
	TObjectPtr<UAnimMontage> GuardBreakMontage;
};
