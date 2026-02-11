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

void UStaminaBarWidget::NativeDestruct()
{
	// Cancel any pending hide timer
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HideTimerHandle);
	}

	Super::NativeDestruct();
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
		if (const UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(HideTimerHandle);
		}

		if (!bIsVisible)
		{
			ShowBar();
		}
	}
	else
	{
		// Stamina is full -- start the hide delay if not already waiting.
		if (bIsVisible && !HideTimerHandle.IsValid())
		{
			if (const UWorld* World = GetWorld())
			{
				World->GetTimerManager().SetTimer(
					HideTimerHandle, this, &UStaminaBarWidget::HideBar, HideDelay, false);
			}
		}
	}
}

void UStaminaBarWidget::ShowBar()
{
	bIsVisible = true;

	// Cancel any pending hide timer
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HideTimerHandle);
	}

	SetRenderOpacity(1.0f);
	UE_LOG(LogSerene, Verbose, TEXT("UStaminaBarWidget::ShowBar - Bar now visible."));

	if (FadeAnimation)
	{
		PlayAnimation(FadeAnimation, 0.0f, 1, EUMGSequencePlayMode::Forward);
	}
}

void UStaminaBarWidget::HideBar()
{
	bIsVisible = false;
	HideTimerHandle.Invalidate();

	SetRenderOpacity(0.0f);

	if (FadeAnimation)
	{
		PlayAnimation(FadeAnimation, 0.0f, 1, EUMGSequencePlayMode::Reverse);
	}
}
