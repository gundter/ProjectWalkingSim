// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SaveLoadMenuWidget.generated.h"

class UButton;
class UTextBlock;
class UWidget;
class USaveSlotWidget;
class USaveSubsystem;

/** Mode for the save/load menu. */
UENUM(BlueprintType)
enum class ESaveLoadMode : uint8
{
	Save,
	Load
};

/** Delegate broadcast when the menu is closed (back button or after save/load completes). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSaveLoadMenuClosed);

/**
 * 3-slot save/load menu used for both saving (tape recorder) and loading (pause menu).
 *
 * C++ base class for a UMG Blueprint subclass (WBP_SaveLoadMenu) with:
 *   - "MenuTitleText"     (UTextBlock)      -- "Save Game" or "Load Game"
 *   - "Slot0"             (USaveSlotWidget)  -- first slot widget
 *   - "Slot1"             (USaveSlotWidget)  -- second slot widget
 *   - "Slot2"             (USaveSlotWidget)  -- third slot widget
 *   - "BackButton"        (UButton)          -- close/cancel button
 *   - "BackButtonText"    (UTextBlock)       -- "Back" or "Cancel"
 *   - "ConfirmOverlay"    (UWidget)          -- overwrite confirmation panel (initially hidden)
 *   - "ConfirmText"       (UTextBlock)       -- "Overwrite this save?"
 *   - "ConfirmYesButton"  (UButton)          -- confirm overwrite
 *   - "ConfirmNoButton"   (UButton)          -- cancel overwrite
 *
 * Usage:
 *   1. Create WBP_SaveLoadMenu in editor (reparent to USaveLoadMenuWidget)
 *   2. Add the required child widgets with matching names
 *   3. Caller creates widget, calls OpenMenu(ESaveLoadMode), binds OnMenuClosed
 *   4. Input mode is managed by the caller (set UI-only when showing)
 */
UCLASS()
class PROJECTWALKINGSIM_API USaveLoadMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Open the menu in Save or Load mode.
	 * Updates title text and refreshes all 3 slot displays.
	 * @param Mode Save or Load
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void OpenMenu(ESaveLoadMode Mode);

	/**
	 * Close the menu and broadcast OnMenuClosed.
	 * Hides the widget. Caller is responsible for input mode changes.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void CloseMenu();

	/** Broadcast when the menu is closed (by back button or after save/load). */
	UPROPERTY(BlueprintAssignable, Category = "Save")
	FOnSaveLoadMenuClosed OnMenuClosed;

protected:
	virtual void NativeConstruct() override;

	/** Menu title: "Save Game" or "Load Game". Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MenuTitleText;

	/** Save slot widgets (0-2). Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USaveSlotWidget> Slot0;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USaveSlotWidget> Slot1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USaveSlotWidget> Slot2;

	/** Close/cancel button. Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BackButton;

	/** Text on the back button. Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BackButtonText;

	// --- Confirmation overlay for overwrite ---

	/** Container for the overwrite confirmation prompt. Initially Collapsed. Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> ConfirmOverlay;

	/** "Overwrite this save?" text. Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ConfirmText;

	/** Confirm overwrite button. Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ConfirmYesButton;

	/** Cancel overwrite button. Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ConfirmNoButton;

private:
	/** Current mode: Save or Load. */
	ESaveLoadMode CurrentMode = ESaveLoadMode::Save;

	/** Slot index awaiting overwrite confirmation (-1 = none pending). */
	int32 PendingOverwriteSlotIndex = -1;

	/** Handler for slot click events. Routes to save or load based on CurrentMode. */
	UFUNCTION()
	void HandleSlotClicked(int32 SlotIndex);

	/** Confirm overwrite: save to PendingOverwriteSlotIndex and close. */
	UFUNCTION()
	void HandleConfirmYes();

	/** Cancel overwrite: hide confirmation overlay. */
	UFUNCTION()
	void HandleConfirmNo();

	/** Back button clicked: close menu and broadcast OnMenuClosed. */
	UFUNCTION()
	void HandleBackClicked();

	/** Refresh all 3 slot widgets from SaveSubsystem data. */
	void RefreshSlots();

	/** Helper: returns all slot widgets for iteration. */
	TArray<USaveSlotWidget*> GetSlotWidgets() const;
};
