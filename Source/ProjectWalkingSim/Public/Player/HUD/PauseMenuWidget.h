// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UButton;
class UTextBlock;
class USaveLoadMenuWidget;

/** Delegate broadcast when the pause menu is closed (Resume pressed). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPauseMenuClosed);

/**
 * Pause menu opened by Esc key.
 *
 * C++ base class for a UMG Blueprint subclass (WBP_PauseMenu) with:
 *   - "PauseTitle"        (UTextBlock) -- "PAUSED"
 *   - "ContinueButton"    (UButton)    -- loads most recent save
 *   - "LoadGameButton"    (UButton)    -- opens SaveLoadMenuWidget in Load mode
 *   - "ResumeButton"      (UButton)    -- closes pause menu, resumes game
 *   - "QuitButton"        (UButton)    -- exits to desktop
 *
 * Continue and Load Game buttons are disabled if no saves exist.
 *
 * Usage:
 *   1. Create WBP_PauseMenu in editor (reparent to UPauseMenuWidget)
 *   2. Add the required widgets with matching names
 *   3. SerenePlayerController creates and manages this widget on Esc key
 */
UCLASS()
class PROJECTWALKINGSIM_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Broadcast when Resume is clicked. Controller uses this to unpause. */
	UPROPERTY(BlueprintAssignable, Category = "Pause")
	FOnPauseMenuClosed OnPauseMenuClosed;

protected:
	virtual void NativeConstruct() override;

	/** "PAUSED" title text. Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PauseTitle;

	/** Button to load the most recent save. Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ContinueButton;

	/** Button to open the save/load menu in Load mode. Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LoadGameButton;

	/** Button to close the pause menu and resume play. Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ResumeButton;

	/** Button to quit the game. Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> QuitButton;

	/** Blueprint class for the load menu (SaveLoadMenuWidget subclass). */
	UPROPERTY(EditDefaultsOnly, Category = "Pause")
	TSubclassOf<USaveLoadMenuWidget> LoadMenuWidgetClass;

private:
	/** Live load menu instance (created when Load Game is pressed). */
	UPROPERTY()
	TObjectPtr<USaveLoadMenuWidget> LoadMenuInstance;

	/** Continue: load the most recent save. */
	UFUNCTION()
	void HandleContinueClicked();

	/** Load Game: open the save/load menu in Load mode. */
	UFUNCTION()
	void HandleLoadGameClicked();

	/** Resume: close pause menu and broadcast OnPauseMenuClosed. */
	UFUNCTION()
	void HandleResumeClicked();

	/** Quit: exit the application. */
	UFUNCTION()
	void HandleQuitClicked();

	/** Called when the load menu is closed. Shows pause menu again. */
	UFUNCTION()
	void HandleLoadMenuClosed();
};
