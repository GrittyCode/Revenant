#include "Character/Base/RVCharacterBase.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVEquipmentComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVHitReactionComponent.h"
#include "Data/RVCharacterDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"

ARVCharacterBase::ARVCharacterBase()
{
    PrimaryActorTick.bCanEverTick = false;

    AttributeComponent   = CreateDefaultSubobject<URVAttributeComponent>  (TEXT("AttributeComponent"));
    EquipmentComponent   = CreateDefaultSubobject<URVEquipmentComponent>   (TEXT("EquipmentComponent"));
    CombatStateComponent = CreateDefaultSubobject<URVCombatStateComponent> (TEXT("CombatStateComponent"));
    HitReactionComponent = CreateDefaultSubobject<URVHitReactionComponent> (TEXT("HitReactionComponent"));
}

void ARVCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    ensureMsgf(IsValid(AttributeComponent),   TEXT("[%s] AttributeComponent missing"),   *GetName());
    ensureMsgf(IsValid(EquipmentComponent),   TEXT("[%s] EquipmentComponent missing"),   *GetName());
    ensureMsgf(IsValid(CombatStateComponent), TEXT("[%s] CombatStateComponent missing"), *GetName());
    ensureMsgf(IsValid(HitReactionComponent), TEXT("[%s] HitReactionComponent missing"), *GetName());

    if (IsValid(CharacterData))
    {
        AttributeComponent->InitFromDataAsset(CharacterData);
    }

    //--- Reference Injection (Composition Root) ------------------------------

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();

    CombatStateComponent->InitReferences(this, EquipmentComponent, MoveComp);
    HitReactionComponent->InitReferences(this, CombatStateComponent, AttributeComponent, EquipmentComponent, CharacterData);
}

//--- IRVCombatInterface ------------------------------------------------------

void ARVCharacterBase::ActivateHitCheck()
{
    CombatStateComponent->PerformAttackTrace();
}

//--- IRVDamageable -----------------------------------------------------------

bool ARVCharacterBase::ApplyDamage(const FRVHitInfo& InHitInfo)
{
    if (CombatStateComponent->IsInvincible()) { return false; }

    const bool bSurvived = AttributeComponent->ApplyDamage(InHitInfo.Instigator, InHitInfo.Damage);
    HitReactionComponent->HandleHit(InHitInfo);

    return bSurvived;
}

//--- Movement ----------------------------------------------------------------

void ARVCharacterBase::Falling()
{
    Super::Falling();

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    OriginalRotationRate   = MoveComp->RotationRate;
    MoveComp->RotationRate = AirRotationRate;
}

void ARVCharacterBase::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);
    GetCharacterMovement()->RotationRate = OriginalRotationRate;
}