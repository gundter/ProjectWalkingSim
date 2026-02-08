// Copyright Null Lantern.

#include "Player/HUD/StaminaBarWidget.h"

#include "Components/ProgressBar.h"
#include "Animation/WidgetAnimation.h"
#include "Core/SereneLogChannels.h"

void UStaminaBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Start hidden -- ShowBar will reveal when stamina drops below 100%.
	SetRenderOpacity(0.0f);
}

void UStaminaBarWidget::SetStaminaPercent(float Percent)
{
	if (StaminaBar)
	{
		StaminaBar->SetPercent(Percent);
	}
	else
	{
		UE_LOG(LogSerene, Warning, TEXT("UStaminaBarWidget::SetStaminaPercent - StaminaBar ProgressBar is null. Check BindWidget name in WBP."));
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

	SetRenderOpacity(1.0f);
	UE_LOG(LogSerene, Log, TEXT("UStaminaBarWidget::ShowBar - Bar now visible."));

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

	SetRenderOpacity(0.0f);

	if (FadeAnimation)
	{
		PlayAnimation(FadeAnimation, 0.0f, 1, EUMGSequencePlayMode::Reverse);
	}
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
