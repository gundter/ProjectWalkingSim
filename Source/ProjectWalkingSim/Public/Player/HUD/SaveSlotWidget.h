// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SaveSlotWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
struct FSaveSlotInfo;

/** Delegate broadcast when a save slot widget is clicked. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveSlotClicked, int32, SlotIndex);

/**
 * Displays a single save slot: screenshot thumbnail, timestamp, and empty/occupied indicator.
 *
 * C++ base class for a UMG Blueprint subclass (WBP_SaveSlot) with:
 *   - "ThumbnailImage" (UImage)     -- screenshot or placeholder
 *   - "TimestampText"  (UTextBlock) -- formatted date or "Empty"
 *   - "SlotLabelText"  (UTextBlock) -- "Slot 1", "Slot 2", "Slot 3"
 *   - "SlotButton"     (UButton)    -- clickable area
 *
 * Usage:
 *   1. Create WBP_SaveSlot in editor (reparent to USaveSlotWidget)
 *   2. Add the required widgets with matching names
 *   3. SaveLoadMenuWidget creates 3 instances and binds OnSlotClicked
 */
UCLASS()
class PROJECTWALKINGSIM_API USaveSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Populate the widget with slot data.
	 * @param InSlotIndex Slot index (0-2)
	 * @param Info Slot metadata (timestamp + screenshot bytes)
	 * @param bIsOccupied True if this slot has a save file
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void SetSlotData(int32 InSlotIndex, const FSaveSlotInfo& Info, bool bIsOccupied);

	/** Returns the cached slot index. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Save")
	int32 GetSlotIndex() const { return SlotIndex; }

	/** Broadcast when the slot button is clicked. */
	UPROPERTY(BlueprintAssignable, Category = "Save")
	FOnSaveSlotClicked OnSlotClicked;

protected:
	virtual void NativeConstruct() override;

	/** Screenshot thumbnail or placeholder. Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ThumbnailImage;

	/** Formatted timestamp or "Empty". Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TimestampText;

	/** Slot label ("Slot 1", etc.). Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SlotLabelText;

	/** Clickable button for the slot. Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SlotButton;

private:
	/** Cached slot index set by SetSlotData. */
	int32 SlotIndex = -1;

	/** Internal handler for SlotButton click. Broadcasts OnSlotClicked. */
	UFUNCTION()
	void HandleSlotButtonClicked();
};
