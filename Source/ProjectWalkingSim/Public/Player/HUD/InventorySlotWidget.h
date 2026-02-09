// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/InventoryTypes.h"
#include "InventorySlotWidget.generated.h"

class UImage;
class UTextBlock;
class UItemDataAsset;

/** Delegate broadcast when a slot widget is clicked. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotClicked, int32, SlotIndex);

/**
 * Inventory slot widget displaying a single item slot.
 *
 * C++ base class that handles item display and selection state.
 * Requires a UMG Blueprint subclass (WBP_InventorySlot) with:
 *   - "SlotBackground"     (UImage)     -- slot background image
 *   - "ItemIcon"           (UImage)     -- item icon display
 *   - "QuantityText"       (UTextBlock) -- quantity badge
 *   - "SelectionHighlight" (UImage)     -- optional selection indicator
 *
 * Usage:
 *   1. Create WBP_InventorySlot in editor (reparent to this class)
 *   2. Add the required widgets with matching names
 *   3. InventoryWidget creates 8 instances and binds OnSlotClicked
 */
UCLASS()
class PROJECTWALKINGSIM_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Update the slot display with item data.
	 * @param SlotData Runtime slot state (ItemId, Quantity)
	 * @param ItemData Item definition for icon/display info
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetSlotData(const FInventorySlot& SlotData, const UItemDataAsset* ItemData);

	/** Clear the slot to empty visual state. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ClearSlot();

	/**
	 * Set the selection state of this slot.
	 * @param bSelected True to show selection highlight
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetSelected(bool bSelected);

	/** Index of this slot in the inventory array. Set by InventoryWidget during creation. */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 SlotIndex = -1;

	/** Broadcast when this slot is clicked. */
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnSlotClicked OnSlotClicked;

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** Slot background image. Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> SlotBackground;

	/** Item icon display. Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIcon;

	/** Quantity text badge. Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> QuantityText;

	/** Optional selection highlight indicator. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> SelectionHighlight;
};
