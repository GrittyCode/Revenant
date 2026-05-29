#include "UI/RVBossHPBarWidget.h"
#include "Character/Enemy/RVSevarogCharacter.h"
#include "Data/ASset/RVSevarogDataAsset.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void URVBossHPBarWidget::SetBoss(ARVSevarogCharacter* InBoss)
{
    if (!IsValid(InBoss)) { return; }

    BossRef = InBoss;

    // [IsValid-8] BindWidget 계약 — BossNameText는 항상 non-null.
    if (IsValid(InBoss->GetSevarogData()))
    {
        BossNameText->SetText(InBoss->GetSevarogData()->BossName);
    }

    // [IsValid-9] BindWidget 계약 — HPBar, PoiseBar는 항상 non-null.
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

// [설계-5] 정규화 비율을 델리게이트에서 직접 받아 사용 — BossRef 재조회 및 IsValid 불필요.
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
    // Hold at 1.0 during groggy — communicates groggy is active.
    PoiseBar->SetPercent(1.f);
}

void URVBossHPBarWidget::OnGroggyEnded()
{
    // Poise has been reset by OnGroggySequenceCompleted → show empty bar.
    PoiseBar->SetPercent(0.f);
}
