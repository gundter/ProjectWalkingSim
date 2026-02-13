// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DemoEndingManager.generated.h"

class UUserWidget;

/**
 * Ending sequence orchestrator component.
 * Disables player input, plays an optional final monologue, fades the screen
 * to black via StartCameraFade, and shows a title card widget after the fade.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTWALKINGSIM_API UDemoEndingManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UDemoEndingManager();

	/** Triggers the demo ending sequence. Safe to call multiple times (double-trigger guard). */
	UFUNCTION(BlueprintCallable, Category = "Demo")
	void TriggerEnding();

protected:
	/** Duration of the fade-to-black in seconds. */
	UPROPERTY(EditAnywhere, Category = "Demo|Ending", meta = (ClampMin = "0.1"))
	float FadeDuration = 3.0f;

	/** Delay after fade completes before showing the title card. */
	UPROPERTY(EditAnywhere, Category = "Demo|Ending", meta = (ClampMin = "0.0"))
	float TitleCardDelay = 2.0f;

	/** Widget class to display as the title card. */
	UPROPERTY(EditDefaultsOnly, Category = "Demo|Ending")
	TSubclassOf<UUserWidget> TitleCardWidgetClass;

	/** Optional final monologue sound to play during the ending. */
	UPROPERTY(EditAnywhere, Category = "Demo|Ending")
	TObjectPtr<USoundBase> FinalMonologue;

private:
	/** Guard against multiple TriggerEnding calls. */
	bool bEndingTriggered = false;

	/** Timer for delayed title card display. */
	UPROPERTY()
	FTimerHandle TitleCardTimerHandle;

	/** Prevents GC of the title card widget instance. */
	UPROPERTY()
	TObjectPtr<UUserWidget> TitleCardInstance;

	/** Shows the title card widget and sets input mode to UI. */
	void ShowTitleCard();
};
