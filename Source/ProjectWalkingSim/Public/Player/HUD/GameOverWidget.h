// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameOverWidget.generated.h"

class UButton;
class UTextBlock;
class USaveSubsystem;

/**
 * Game Over death screen displayed when the Wendigo kills the player.
 *
 * C++ base class with full logic; requires a UMG Blueprint subclass (WBP_GameOver) with:
 *   - "GameOverText"           (UTextBlock) -- "GAME OVER" title
 *   - "LoadLastSaveButton"     (UButton)    -- loads most recent save or restarts
 *   - "LoadLastSaveButtonText" (UTextBlock) -- dynamic text: "Load Last Save" / "Restart"
 *   - "QuitButton"             (UButton)    -- exits to desktop
 *
 * Behavior:
 *   - On construct, checks SaveSubsystem->HasAnySave() to decide button text.
 *   - Load button either loads latest save or restarts the current level.
 *   - Quit button exits the application.
 *
 * Usage:
 *   1. Create WBP_GameOver in editor (reparent to UGameOverWidget)
 *   2. Add the required widgets with matching names
 *   3. SereneGameMode::OnPlayerDeath creates and adds this widget to viewport
 */
UCLASS()
class PROJECTWALKINGSIM_API UGameOverWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	/** "GAME OVER" title text. Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GameOverText;

	/** Button to load the most recent save or restart. Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LoadLastSaveButton;

	/** Text on the load button. Dynamically set to "Load Last Save" or "Restart". */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LoadLastSaveButtonText;

	/** Button to quit the game. Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> QuitButton;

private:
	/** Handler for LoadLastSaveButton click. Loads latest save or restarts level. */
	UFUNCTION()
	void HandleLoadClicked();

	/** Handler for QuitButton click. Exits the application. */
	UFUNCTION()
	void HandleQuitClicked();
};
