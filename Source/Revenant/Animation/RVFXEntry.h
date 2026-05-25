#pragma once

#include "CoreMinimal.h"
#include "RVFXEntry.generated.h"

class UParticleSystem;

USTRUCT(BlueprintType)
struct FRVFXEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "RV|FX")
    TObjectPtr<UParticleSystem> FX;

    // Attach to this socket. If None, attaches to mesh root.
    UPROPERTY(EditAnywhere, Category = "RV|FX")
    FName SocketName = NAME_None;

    UPROPERTY(EditAnywhere, Category = "RV|FX")
    FVector LocationOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "RV|FX")
    FRotator RotationOffset = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, Category = "RV|FX")
    FVector Scale = FVector::OneVector;
};
