// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/HUD/ItemTooltipWidget.h"
#include "InventoryWidget.generated.h"

class UHorizontalBox;
class UInventorySlotWidget;
class UItemTooltipWidget;
class UInventoryComponent;
struct FInventorySlot;

/**
 * Root inventory panel widget containing slot grid and tooltip.
 *
 * C++ base class that creates and manages 8 slot widgets in a horizontal row.
 * Requires a UMG Blueprint subclass (WBP_Inventory) with:
 *   - "SlotContainer"  (UHorizontalBox)      -- horizontal box for slot widgets
 *   - "ItemTooltip"    (UItemTooltipWidget)  -- tooltip widget instance
 *
 * The widget starts hidden (opacity 0) and manages:
 *   - Dynamic creation of 8 InventorySlotWidget instances
 *   - Slot selection state and toggle behavior
 *   - Tooltip visibility and data binding
 *   - Re-broadcasting tooltip action events
 *
 * Usage:
 *   1. Create WBP_Inventory in editor (reparent to this class)
 *   2. Add HorizontalBox named "SlotContainer"
 *   3. Add WBP_ItemTooltip child named "ItemTooltip"
 *   4. Set SlotWidgetClass to WBP_InventorySlot in Blueprint defaults
 *   5. SereneHUD binds InventoryComponent::OnInventoryChanged to RefreshSlots
 */
UCLASS()
class PROJECTWALKINGSIM_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Refresh all slot displays from inventory data.
	 * @param Slots Array of slot states from InventoryComponent
	 * @param InventoryComp Inventory component for item data lookup
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RefreshSlots(const TArray<FInventorySlot>& Slots, const UInventoryComponent* InventoryComp);

	/** Show the inventory panel (set opacity to 1). */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ShowInventory();

	/** Hide the inventory panel (set opacity to 0, deselect slot, hide tooltip). */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void HideInventory();

	/** Returns true if inventory panel is currently visible. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	bool IsInventoryVisible() const { return bIsVisible; }

	/**
	 * Select a slot by index.
	 * @param SlotIndex Slot to select (0-7)
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SelectSlot(int32 SlotIndex);

	/** Deselect the current slot and hide tooltip. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void DeselectSlot();

	/** Get currently selected slot index (-1 if none). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 GetSelectedSlotIndex() const { return SelectedSlotIndex; }

	// --- Delegate forwarding (tooltip actions re-broadcast for HUD routing) ---

	/** Broadcast when Use action is requested for a slot. */
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnTooltipAction OnUseRequested;

	/** Broadcast when Combine action is requested (enters combine mode). */
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnTooltipAction OnCombineRequested;

	/** Broadcast when Discard action is requested for a slot. */
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnTooltipAction OnDiscardRequested;

	/** Broadcast when combine mode completes with both slots selected. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombineSlotSelected, int32, SlotIndexA, int32, SlotIndexB);

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnCombineSlotSelected OnCombineSlotSelected;

	// --- Combine Mode ---

	/** Enter combine mode: first item selected, waiting for second. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void EnterCombineMode(int32 SourceSlotIndex);

	/** Exit combine mode without combining. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ExitCombineMode();

	/** Returns true if in combine mode. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	bool IsInCombineMode() const { return bInCombineMode; }

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	/** Horizontal box container for slot widgets. Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> SlotContainer;

	/** Tooltip widget instance. Must exist in UMG Blueprint as "ItemTooltip". */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UItemTooltipWidget> ItemTooltip;

	/** Widget class to use for slot creation. Set to WBP_InventorySlot in Blueprint defaults. */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UInventorySlotWidget> SlotWidgetClass;

private:
	/** Array of created slot widget instances. */
	UPROPERTY()
	TArray<TObjectPtr<UInventorySlotWidget>> SlotWidgets;

	/** Currently selected slot index (-1 = none). */
	int32 SelectedSlotIndex = -1;

	/** Whether inventory panel is currently visible. */
	bool bIsVisible = false;

	/** Whether in combine mode (waiting for second slot selection). */
	bool bInCombineMode = false;

	/** Source slot index for combine mode. */
	int32 CombineSourceSlotIndex = -1;

	/** Cached reference to inventory component for data lookup. */
	UPROPERTY()
	TObjectPtr<const UInventoryComponent> CachedInventoryComp;

	/** Internal handler for slot click events. */
	UFUNCTION()
	void HandleSlotClicked(int32 SlotIndex);

	/** Internal handlers for tooltip action re-broadcasting. */
	UFUNCTION()
	void HandleUseClicked(int32 SlotIndex);

	UFUNCTION()
	void HandleCombineClicked(int32 SlotIndex);

	UFUNCTION()
	void HandleDiscardClicked(int32 SlotIndex);

	/** Navigate slots left (-1) or right (+1) with wraparound. */
	void NavigateSlot(int32 Direction);
};
