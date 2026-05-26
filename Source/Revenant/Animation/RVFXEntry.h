#pragma once

#include "CoreMinimal.h"
#include "RVFXEntry.generated.h"

class UParticleSystem;
class UNiagaraSystem;
class USoundBase;

USTRUCT(BlueprintType)
struct FRVFXEntry
{
	GENERATED_BODY()

	// Cascade particle. Used when NiagaraFX is null.
	UPROPERTY(EditAnywhere, Category = "RV|FX")
	TObjectPtr<UParticleSystem> FX;

	// Niagara system. Takes priority over FX when both are set.
	UPROPERTY(EditAnywhere, Category = "RV|FX")
	TObjectPtr<UNiagaraSystem> NiagaraFX;

	// Optional SFX played at the spawn location.
	UPROPERTY(EditAnywhere, Category = "RV|FX")
	TObjectPtr<USoundBase> SFX;

	// Attach to this socket. NAME_None attaches to mesh root.
	UPROPERTY(EditAnywhere, Category = "RV|FX")
	FName SocketName = NAME_None;

	UPROPERTY(EditAnywhere, Category = "RV|FX")
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "RV|FX")
	FRotator RotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category = "RV|FX")
	FVector Scale = FVector::OneVector;
};