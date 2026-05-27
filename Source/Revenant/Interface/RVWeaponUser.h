#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RVWeaponUser.generated.h"

class URVWeaponDataAsset;

// Implemented by any Actor that equips a weapon and can supply WeaponDataAsset.
// Components requiring weapon data Cast<IRVWeaponUser>(GetOwner()) — never cast to a concrete class.
// Pattern mirrors GAS IAbilitySystemInterface: interface call only, no concrete type dependency.
UINTERFACE(MinimalAPI, NotBlueprintable)
class URVWeaponUser : public UInterface
{
    GENERATED_BODY()
};

class REVENANT_API IRVWeaponUser
{
    GENERATED_BODY()

public:
    virtual URVWeaponDataAsset* GetCurrentWeaponData() const = 0;
};
