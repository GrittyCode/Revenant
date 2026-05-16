// Source/Revenant/Character/Base/RVCharacterBase.cpp
#include "Character/Base/RVCharacterBase.h"
#include "Component/RVAttributeComponent.h"
#include "Component/RVCombatStateComponent.h"
#include "Component/RVHitReactionComponent.h"
#include "Components/CapsuleComponent.h"
#include "Data/RVCharacterDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"

ARVCharacterBase::ARVCharacterBase()
{
    PrimaryActorTick.bCanEverTick = false;

    AttributeComponent   = CreateDefaultSubobject<URVAttributeComponent>  (TEXT("AttributeComponent"));
    CombatStateComponent = CreateDefaultSubobject<URVCombatStateComponent> (TEXT("CombatStateComponent"));
    HitReactionComponent = CreateDefaultSubobject<URVHitReactionComponent> (TEXT("HitReactionComponent"));
}

void ARVCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

    ensureMsgf(IsValid(AttributeComponent),   TEXT("[%s] AttributeComponent missing"),   *GetName());
    ensureMsgf(IsValid(CombatStateComponent), TEXT("[%s] CombatStateComponent missing"), *GetName());
    ensureMsgf(IsValid(HitReactionComponent), TEXT("[%s] HitReactionComponent missing"), *GetName());

    if (IsValid(CharacterData))
    {
        AttributeComponent->InitFromDataAsset(CharacterData);
    }

    //--- Reference Injection (Composition Root) ------------------------------
    // Virtual dispatch: Player returns EquipmentComponent data, Boss returns BossData.

    URVCombatDataAsset* CombatData = GetCombatData();
    UMeshComponent*     TraceMesh  = GetWeaponTraceMesh();

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();

    CombatStateComponent->InitReferences(this, CombatData, TraceMesh, MoveComp);
    HitReactionComponent->InitReferences(this, CombatStateComponent, AttributeComponent, CombatData, CharacterData);
}

void ARVCharacterBase::ActivateHitCheck()
{
    CombatStateComponent->PerformAttackTrace();
}

bool ARVCharacterBase::ApplyDamage(const FRVHitInfo& InHitInfo)
{
    if (CombatStateComponent->IsInvincible()) { return false; }

    const bool bSurvived = AttributeComponent->ApplyDamage(InHitInfo.Instigator, InHitInfo.Damage);

    // HandleHit runs even on death — Knockdown montage serves as the death fall animation.
    // TODO: guard with bSurvived once OnDeath handler is wired.
    HitReactionComponent->HandleHit(InHitInfo);

    return bSurvived;
}

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