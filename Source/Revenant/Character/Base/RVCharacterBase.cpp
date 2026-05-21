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

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw   = false;
	bUseControllerRotationRoll  = false;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->bOrientRotationToMovement = true;
	MoveComp->RotationRate = FRotator(0.f, 500.f, 0.f);

	
	GetCapsuleComponent()->CanCharacterStepUpOn = ECB_No;
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

	// Player: CharacterData assigned in BP → initializes HP/Stamina/Poise.
	// Boss: CharacterData is null → skipped here, initialized from DT_EnemyStats in ARVSevarogCharacter::BeginPlay.
	if (IsValid(CharacterData))
	{
		AttributeComponent->InitFromDataAsset(CharacterData);
	}

	//--- Reference Injection (Composition Root) ------------------------------

	URVHitReactionAnimDataAsset* HitReactionAnimData = GetHitReactionAnimData();
	UMeshComponent*              TraceMesh           = GetWeaponTraceMesh();
	UCharacterMovementComponent* MoveComp            = GetCharacterMovement();

	CombatStateComponent->InitReferences(this, TraceMesh, MoveComp);

	// StaggerDuration: player uses CharacterData value; boss uses DT_EnemyStats (overrides after Super).
	const float StaggerDuration = IsValid(CharacterData) ? CharacterData->StaggerDuration : 0.5f;
	HitReactionComponent->InitReferences(this, CombatStateComponent, AttributeComponent, HitReactionAnimData, StaggerDuration);
	
	
	AttributeComponent->OnDeath.AddDynamic(this, &ARVCharacterBase::OnDeath);
}

void ARVCharacterBase::ActivateHitCheck()
{
	CombatStateComponent->PerformAttackTrace();
}

bool ARVCharacterBase::ApplyDamage(const FRVHitInfo& InHitInfo)
{
	if (CombatStateComponent->IsInvincible()) { return false; }

	const bool bSurvived = AttributeComponent->ApplyDamage(InHitInfo.Instigator, InHitInfo.Damage);

	// On death: URVAttributeComponent broadcasts OnDeath.
	// ARVSevarogCharacter::OnBossDeath / ARVCharacterPlayer::OnPlayerDeath handle cleanup.
	if (bSurvived)
	{
		HitReactionComponent->HandleHit(InHitInfo);
	}

	return bSurvived;
}

float ARVCharacterBase::GetHealthRatio() const
{
	return AttributeComponent->GetHealthPercent();
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


void ARVCharacterBase::OnDeath()
{
}
