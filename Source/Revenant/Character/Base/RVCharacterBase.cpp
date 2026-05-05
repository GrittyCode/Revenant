// Source/Revenant/Character/Base/RVCharacterBase.cpp
#include "Character/Base/RVCharacterBase.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVComboComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Component/RVCombatComponent.h"
#include "Data/RVCharacterDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"

ARVCharacterBase::ARVCharacterBase()
{
    PrimaryActorTick.bCanEverTick = false;
	
    AttributeComponent  = CreateDefaultSubobject<URVAttributeComponent> (TEXT("AttributeComponent"));
    ComboComponent      = CreateDefaultSubobject<URVComboComponent>      (TEXT("ComboComponent"));
    EquipmentComponent  = CreateDefaultSubobject<URVEquipmentComponent>  (TEXT("EquipmentComponent"));
    CombatComponent     = CreateDefaultSubobject<URVCombatComponent>     (TEXT("CombatComponent"));
}

void ARVCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    // Propagate DataAsset values to AttributeComponent before any component BeginPlay reads them
    if (IsValid(CharacterData) && IsValid(AttributeComponent))
    {
        AttributeComponent->InitFromDataAsset(CharacterData);
    }
}

// --- IRVCombatInterface -----------------------------------------------------------

void ARVCharacterBase::ActivateHitCheck()
{
    if (IsValid(CombatComponent))
    {
        CombatComponent->PerformAttackTrace();
    }
}

// --- IRVDamageable -------------------------------------------------------------

bool ARVCharacterBase::ApplyDamage(float InDamageAmount, AActor* InInstigator)
{
    if (!IsValid(AttributeComponent) || !IsValid(CombatComponent)) { return false; }

    // Dodge i-frame — all damage blocked
    if (CombatComponent->IsInvincible()) { return false; }

    // Guarding — absorbed as stamina damage; may trigger guard break
    if (CombatComponent->IsGuarding())
    {
        CombatComponent->HandleGuardHit(InDamageAmount);
        return true; // character is alive regardless of whether guard held or broke
    }

    // Normal hit
    return AttributeComponent->ApplyDamage(InInstigator, InDamageAmount);
}

void ARVCharacterBase::OnHitReaction(FVector InHitDirection)
{
}