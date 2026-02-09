// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemTooltipWidget.generated.h"

class UTextBlock;
class UButton;
class UItemDataAsset;

/** Delegate broadcast when a tooltip action button is clicked. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTooltipAction, int32, SlotIndex);

/**
 * Item tooltip widget displaying item details and action buttons.
 *
 * C++ base class that handles tooltip data and action button events.
 * Requires a UMG Blueprint subclass (WBP_ItemTooltip) with:
 *   - "ItemNameText"        (UTextBlock) -- item name display
 *   - "ItemDescriptionText" (UTextBlock) -- item description display
 *   - "UseButton"           (UButton)    -- use/consume action
 *   - "CombineButton"       (UButton)    -- combine action
 *   - "InfoButton"          (UButton)    -- info/examine action (optional)
 *   - "DiscardButton"       (UButton)    -- discard action
 *
 * Usage:
 *   1. Create WBP_ItemTooltip in editor (reparent to this class)
 *   2. Add the required widgets with matching names
 *   3. InventoryWidget manages tooltip visibility and data binding
 */
UCLASS()
class PROJECTWALKINGSIM_API UItemTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Update the tooltip with item data.
	 * @param InSlotIndex The slot this tooltip is showing for
	 * @param ItemData Item definition for name/description
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetTooltipData(int32 InSlotIndex, const UItemDataAsset* ItemData);

	/** Hide the tooltip (set render opacity to 0). */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void HideTooltip();

	/** Show the tooltip (set render opacity to 1). */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ShowTooltip();

	/** Which slot this tooltip is currently showing for. */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 BoundSlotIndex = -1;

	/** Broadcast when Use button is clicked. */
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnTooltipAction OnUseClicked;

	/** Broadcast when Combine button is clicked. */
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnTooltipAction OnCombineClicked;

	/** Broadcast when Info button is clicked. */
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnTooltipAction OnInfoClicked;

	/** Broadcast when Discard button is clicked. */
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnTooltipAction OnDiscardClicked;

protected:
	virtual void NativeConstruct() override;

	/** Item name text display. Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemNameText;

	/** Item description text display. Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemDescriptionText;

	/** Use action button. Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> UseButton;

	/** Combine action button. Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CombineButton;

	/** Info/examine action button. Optional. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> InfoButton;

	/** Discard action button. Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> DiscardButton;

private:
	/** Internal handler for Use button click. */
	UFUNCTION()
	void HandleUseClicked();

	/** Internal handler for Combine button click. */
	UFUNCTION()
	void HandleCombineClicked();

	/** Internal handler for Info button click. */
	UFUNCTION()
	void HandleInfoClicked();

	/** Internal handler for Discard button click. */
	UFUNCTION()
	void HandleDiscardClicked();
};
