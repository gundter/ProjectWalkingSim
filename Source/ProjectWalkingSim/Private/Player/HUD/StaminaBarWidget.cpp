// Copyright Null Lantern.

#include "Player/HUD/StaminaBarWidget.h"

#include "Components/ProgressBar.h"
#include "Animation/WidgetAnimation.h"

void UStaminaBarWidget::SetStaminaPercent(float Percent)
{
	if (StaminaBar)
	{
		StaminaBar->SetPercent(Percent);
	}

	if (Percent < 1.0f)
	{
		// Stamina is not full -- show the bar and cancel any hide timer.
		bWaitingToHide = false;
		HideTimer = 0.0f;

		if (!bIsVisible)
		{
			ShowBar();
		}
	}
	else
	{
		// Stamina is full -- start the hide delay if not already waiting.
		if (bIsVisible && !bWaitingToHide)
		{
			bWaitingToHide = true;
			HideTimer = 0.0f;
		}
	}
}

void UStaminaBarWidget::ShowBar()
{
	bIsVisible = true;
	bWaitingToHide = false;
	HideTimer = 0.0f;

	SetVisibility(ESlateVisibility::Visible);

	if (FadeAnimation)
	{
		PlayAnimation(FadeAnimation, 0.0f, 1, EUMGSequencePlayMode::Forward);
	}
}

void UStaminaBarWidget::HideBar()
{
	bIsVisible = false;
	bWaitingToHide = false;
	HideTimer = 0.0f;

	if (FadeAnimation)
	{
		PlayAnimation(FadeAnimation, 0.0f, 1, EUMGSequencePlayMode::Reverse);
		// After reverse animation completes, widget is visually hidden.
		// We still collapse it for hit-test purposes.
	}

	SetVisibility(ESlateVisibility::Collapsed);
}

void UStaminaBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bWaitingToHide)
	{
		HideTimer += InDeltaTime;
		if (HideTimer >= HideDelay)
		{
			HideBar();
		}
	}
}
