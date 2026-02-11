// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "StaminaBarWidget.generated.h"

class UProgressBar;
class UWidgetAnimation;

/**
 * HUD widget displaying the stamina progress bar.
 *
 * C++ base class that handles show/hide logic driven by stamina percentage.
 * Requires a UMG Blueprint subclass (WBP_StaminaBar) with a UProgressBar
 * widget named exactly "StaminaBar".
 *
 * Behavior:
 * - Shows when stamina drops below 100% (sprint drain or recovery).
 * - Auto-hides 2 seconds after stamina returns to 100%.
 * - Optionally plays a FadeAnimation (BindWidgetAnimOptional) for smooth transitions.
 *
 * Usage:
 *   1. Create WBP_StaminaBar in the editor (reparent to UStaminaBarWidget).
 *   2. Add a ProgressBar widget named "StaminaBar".
 *   3. Optionally add a UWidgetAnimation named "FadeAnimation" for show/hide.
 *   4. SereneHUD binds StaminaComponent::OnStaminaChanged to SetStaminaPercent().
 */
UCLASS()
class PROJECTWALKINGSIM_API UStaminaBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Update the stamina bar fill amount.
	 * Shows the bar when Percent < 1.0, starts hide timer when Percent >= 1.0.
	 *
	 * @param Percent  Stamina ratio (0.0 = empty, 1.0 = full).
	 */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void SetStaminaPercent(float Percent);

	/** Immediately show the stamina bar. Plays FadeAnimation forward if available. */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void ShowBar();

	/** Immediately hide the stamina bar. Plays FadeAnimation reverse if available. */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void HideBar();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** The progress bar fill. Must exist in UMG Blueprint with this exact name. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaBar;

	/** Optional fade animation for smooth show/hide transitions. */
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> FadeAnimation;

private:
	/** Whether the bar is currently visible (or transitioning to visible). */
	bool bIsVisible = false;

	/** Seconds to wait after stamina reaches 100% before hiding. */
	float HideDelay = 2.0f;

	/** Timer handle for delayed hide after stamina reaches full. */
	FTimerHandle HideTimerHandle;
};
