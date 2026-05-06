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

    // Components are created via CreateDefaultSubobject — always valid on well-formed subclasses.
    // ensureMsgf here catches accidental DestroyComponent calls in subclass constructors.
    ensureMsgf(IsValid(AttributeComponent), TEXT("[%s] AttributeComponent missing"), *GetName());
    ensureMsgf(IsValid(CombatComponent),    TEXT("[%s] CombatComponent missing"),    *GetName());

    // CharacterData is designer-assigned — intentionally not ensured, graceful fallback is correct
    if (IsValid(CharacterData))
    {
        AttributeComponent->InitFromDataAsset(CharacterData);
    }
}

// --- IRVCombatInterface ------------------------------------------------------

void ARVCharacterBase::ActivateHitCheck()
{
    CombatComponent->PerformAttackTrace();
}

// --- IRVDamageable -----------------------------------------------------------

bool ARVCharacterBase::ApplyDamage(float InDamageAmount, AActor* InInstigator)
{
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

// --- Movement ----------------------------------------------------------------

void ARVCharacterBase::Falling()
{
    Super::Falling();

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();

    // Cache current rate before overwriting — restores correctly even if sprint or
    // other systems have modified RotationRate before the jump.
    OriginalRotationRate = MoveComp->RotationRate;
    MoveComp->RotationRate = AirRotationRate;
}

void ARVCharacterBase::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);

    // Restore the exact rate that was active before the jump, not a hardcoded default.
    GetCharacterMovement()->RotationRate = OriginalRotationRate;
}