#include "UI/RVBossHPBarWidget.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "Data/RVSevarogDataAsset.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void URVBossHPBarWidget::SetBoss(ARVSevarogCharacter* InBoss)
{
	if (!IsValid(InBoss)) { return; }

	BossRef = InBoss;

	if (BossNameText && IsValid(InBoss->GetSevarogData()))
	{
		BossNameText->SetText(InBoss->GetSevarogData()->BossName);
	}

	if (HPBar)    { HPBar->SetPercent(InBoss->GetHealthRatio()); }
	if (PoiseBar) { PoiseBar->SetPercent(0.f); }

	InBoss->GetOnHealthChanged().AddDynamic(this, &URVBossHPBarWidget::OnBossHealthChanged);
	InBoss->GetOnPoiseChanged().AddDynamic(this, &URVBossHPBarWidget::OnBossPoiseChangedHandler);
	InBoss->OnBossGroggyStarted.AddUObject(this, &URVBossHPBarWidget::OnGroggyStarted);
	InBoss->OnBossGroggyEnded.AddUObject  (this, &URVBossHPBarWidget::OnGroggyEnded);
}

void URVBossHPBarWidget::NativeDestruct()
{
	if (BossRef.IsValid())
	{
		BossRef->GetOnHealthChanged().RemoveDynamic(this, &URVBossHPBarWidget::OnBossHealthChanged);
		BossRef->GetOnPoiseChanged().RemoveDynamic(this, &URVBossHPBarWidget::OnBossPoiseChangedHandler);
		BossRef->OnBossGroggyStarted.RemoveAll(this);
		BossRef->OnBossGroggyEnded.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void URVBossHPBarWidget::OnBossHealthChanged(float NewHealth, float InDelta)
{
    if (!BossRef.IsValid()) { return; }
    if (HPBar) { HPBar->SetPercent(BossRef->GetHealthRatio()); }
}

void URVBossHPBarWidget::OnBossPoiseChangedHandler(float NewPoiseRatio)
{
	// Invert: full poise = bar empty, zero poise = bar full.
	if (PoiseBar) { PoiseBar->SetPercent(1.f - NewPoiseRatio); }
}

void URVBossHPBarWidget::OnGroggyStarted()
{
    // Hold at 1.0 during groggy — communicates groggy is active.
    if (PoiseBar) { PoiseBar->SetPercent(1.f); }
}

void URVBossHPBarWidget::OnGroggyEnded()
{
    // Poise has been reset by OnGroggySequenceCompleted → show empty bar.
    if (PoiseBar) { PoiseBar->SetPercent(0.f); }
}