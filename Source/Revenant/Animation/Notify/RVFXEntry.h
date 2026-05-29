#pragma once

#include "CoreMinimal.h"
#include "RVFXEntry.generated.h"

class UParticleSystem;
class UNiagaraSystem;
class USoundBase;

// Entry for a Niagara VFX spawn.
// SFX is optional — plays once at the socket location when the FX spawns.
USTRUCT(BlueprintType)
struct FRVNiagaraFXEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "RV|FX")
	TObjectPtr<UNiagaraSystem> NiagaraFX;

	// Optional SFX played at socket location on spawn.
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

// Entry for a Cascade (legacy particle) VFX spawn.
// SFX is optional — plays once at the socket location when the FX spawns.
USTRUCT(BlueprintType)
struct FRVCascadeFXEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "RV|FX")
	TObjectPtr<UParticleSystem> FX;

	// Optional SFX played at socket location on spawn.
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