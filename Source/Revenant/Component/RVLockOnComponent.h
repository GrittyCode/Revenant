#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RVLockOnComponent.generated.h"

class ACharacter;
class APlayerController;
class URVCombatStateComponent;

UCLASS(ClassGroup=(Revenant), meta=(BlueprintSpawnableComponent))
class REVENANT_API URVLockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URVLockOnComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

	void ToggleLockOn();
	void BreakLockOn();

	bool IsLockedOn() const { return bIsLockedOn; }
	AActor* GetLockOnTarget() const;

	void InitReferences(ACharacter* InOwnerCharacter,
						APlayerController* InPlayerController,
						URVCombatStateComponent* InCombatStateComponent);

private:
	AActor* TryFindTarget() const;
	void UpdateCamera(float DeltaTime) const;
	void UpdateCharacterRotation(float DeltaTime) const;

	UPROPERTY(EditDefaultsOnly, Category = "RV|LockOn")
	float LockOnRange = 1500.f;

	UPROPERTY(EditDefaultsOnly, Category = "RV|LockOn")
	float LockOnSearchHalfAngle = 60.f;

	UPROPERTY(EditDefaultsOnly, Category = "RV|LockOn")
	float AutoBreakRange = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category = "RV|LockOn")
	float CameraInterpSpeed = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "RV|LockOn")
	float CharacterRotationInterpSpeed = 15.f;

	bool bIsLockedOn = false;

	TWeakObjectPtr<AActor> LockOnTarget;

	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY()
	TObjectPtr<URVCombatStateComponent> CombatStateComponent;
};