#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RVPlayerCombatAnimDataAsset.generated.h"

class UAnimMontage;

// BlueprintType required: used as TMap key in editor for LockOnDodgeMontages.
UENUM(BlueprintType)
enum class ERVDodgeDirection : uint8
{
	Forward   UMETA(DisplayName = "Forward"),
	Left      UMETA(DisplayName = "Left"),
	Right     UMETA(DisplayName = "Right"),
	BackLeft  UMETA(DisplayName = "Back Left"),
	BackRight UMETA(DisplayName = "Back Right"),
};

UCLASS()
class REVENANT_API URVPlayerCombatAnimDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	//--- Combo ---------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "Combo")
	TArray<TObjectPtr<UAnimMontage>> ComboMontages;

	UAnimMontage* GetComboMontage(int32 InIndex) const;
	int32 GetMaxComboCount() const { return ComboMontages.Num(); }
	int32 FindComboMontageIndex(const UAnimMontage* InMontage) const;

	//--- Heavy Attack --------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "HeavyAttack")
	TObjectPtr<UAnimMontage> HeavyChargeMontage;

	UPROPERTY(EditDefaultsOnly, Category = "HeavyAttack")
	TObjectPtr<UAnimMontage> HeavyAttackMontage;

	// Played when charge completes without manual release (max charge).
	UPROPERTY(EditDefaultsOnly, Category = "HeavyAttack")
	TObjectPtr<UAnimMontage> MaxHeavyAttackMontage;

	//--- Dodge ---------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	TObjectPtr<UAnimMontage> DodgeMontage;

	// Lock-on dodge montages keyed by dodge direction.
	// Falls back to DodgeMontage if a direction entry is missing or unassigned.
	UPROPERTY(EditDefaultsOnly, Category = "Dodge|LockOn")
	TMap<ERVDodgeDirection, TObjectPtr<UAnimMontage>> LockOnDodgeMontages;

	//--- Run Attack ----------------------------------------------------------

	// Played instead of the combo when the character is moving above RunAttackSpeedThreshold.
	UPROPERTY(EditDefaultsOnly, Category = "RunAttack")
	TObjectPtr<UAnimMontage> RunAttackMontage;

	//--- Jump Attack ---------------------------------------------------------

	// Played once while airborne. Resets on landing.
	UPROPERTY(EditDefaultsOnly, Category = "JumpAttack")
	TObjectPtr<UAnimMontage> JumpAttackMontage;

	//--- Guard ---------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "Guard")
	TObjectPtr<UAnimMontage> GuardHitMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Guard")
	TObjectPtr<UAnimMontage> GuardBreakMontage;
};