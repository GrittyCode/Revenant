#include "UI/RVBossHPBarWidget.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "Data/Asset/RVSevarogDataAsset.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void URVBossHPBarWidget::SetBoss(ARVSevarogCharacter* InBoss)
{
    if (!IsValid(InBoss)) { return; }

    BossRef = InBoss;

    // BindWidget contract guarantees BossNameText, HPBar, PoiseBar are non-null.
    if (IsValid(InBoss->GetSevarogData()))
    {
        BossNameText->SetText(InBoss->GetSevarogData()->BossName);
    }

    HPBar->SetPercent(InBoss->GetHealthRatio());
    PoiseBar->SetPercent(0.f);

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

void URVBossHPBarWidget::OnBossHealthChanged(float NewHealthRatio)
{
    HPBar->SetPercent(NewHealthRatio);
}

void URVBossHPBarWidget::OnBossPoiseChangedHandler(float NewPoiseRatio)
{
    // Invert: full poise = bar empty, zero poise = bar full.
    PoiseBar->SetPercent(1.f - NewPoiseRatio);
}

void URVBossHPBarWidget::OnGroggyStarted()
{
    PoiseBar->SetPercent(1.f);
}

void URVBossHPBarWidget::OnGroggyEnded()
{
    PoiseBar->SetPercent(0.f);
}