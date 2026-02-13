// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/InventoryTypes.h"
#include "InventoryComponent.generated.h"

class UItemDataAsset;
class APickupActor;

/**
 * Inventory component managing an 8-slot item system for The Juniper Tree.
 *
 * Attach to ASereneCharacter to provide item storage. The component maintains
 * an array of FInventorySlot structs and a registry mapping ItemId (FName) to
 * UItemDataAsset instances for metadata lookup.
 *
 * On BeginPlay, the component scans the Asset Manager for all "Item" type
 * primary assets and caches them in ItemRegistry for O(1) lookup by ItemId.
 *
 * All slot modifications broadcast OnInventoryChanged for UI updates.
 * Failed operations (e.g., inventory full) broadcast OnInventoryActionFailed.
 *
 * Key operations:
 * - TryAddItem: Add item to inventory (stacks if applicable)
 * - RemoveItem: Remove quantity from specific slot
 * - DiscardItem: Spawn item into world and remove from inventory
 * - GetItemData: Lookup item definition by ItemId
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTWALKINGSIM_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	virtual void BeginPlay() override;

	/** Maximum number of inventory slots. */
	static constexpr int32 MaxSlots = 8;

	// --- Public API ---

	/**
	 * Try to add an item to the inventory.
	 * Stackable items consolidate into existing slots before using new ones.
	 * @param ItemId Identifier matching UItemDataAsset::ItemId
	 * @param Quantity Number of items to add (default 1)
	 * @return True if all items were added, false if inventory full or item not in registry
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool TryAddItem(FName ItemId, int32 Quantity = 1);

	/**
	 * Remove quantity from a specific slot.
	 * Clears the slot if quantity reaches zero.
	 * @param SlotIndex Slot array index (0 to MaxSlots-1)
	 * @param Quantity Number to remove (default 1, use slot's quantity to remove all)
	 * @return True if removal succeeded, false if invalid slot or insufficient quantity
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(int32 SlotIndex, int32 Quantity = 1);

	/**
	 * Remove quantity of a specific item by ItemId.
	 * Finds the first slot containing the item and removes from there.
	 * @param ItemId Identifier to search for
	 * @param Quantity Number to remove (default 1)
	 * @return True if removal succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItemByName(FName ItemId, int32 Quantity = 1);

	/**
	 * Check if the inventory contains at least one of the specified item.
	 * @param ItemId Identifier to search for
	 * @return True if item exists in any slot
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	bool HasItem(FName ItemId) const;

	/**
	 * Get total quantity of a specific item across all slots.
	 * @param ItemId Identifier to count
	 * @return Total quantity (0 if not found)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 GetItemCount(FName ItemId) const;

	/**
	 * Check if all slots are occupied (non-empty).
	 * @return True if no empty slots available
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	bool IsFull() const;

	/**
	 * Get read-only access to all inventory slots.
	 * @return Reference to the slots array
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	const TArray<FInventorySlot>& GetSlots() const { return Slots; }

	/**
	 * Lookup item definition by ItemId.
	 * @param ItemId Identifier to lookup
	 * @return Pointer to item data asset, nullptr if not registered
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	const UItemDataAsset* GetItemData(FName ItemId) const;

	/**
	 * Discard an item from inventory, spawning it into the world.
	 * Spawns an APickupActor in front of the player with a small forward impulse.
	 * @param SlotIndex Slot array index (0 to MaxSlots-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void DiscardItem(int32 SlotIndex);

	/**
	 * Find the first empty slot.
	 * @return Slot index, or -1 if inventory is full
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 FindFirstEmptySlot() const;

	/**
	 * Find the first slot containing the specified item.
	 * @param ItemId Identifier to search for
	 * @return Slot index, or -1 if not found
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 FindSlotWithItem(FName ItemId) const;

	/**
	 * Attempt to combine items from two slots.
	 * @param SlotIndexA First item slot
	 * @param SlotIndexB Second item slot
	 * @return true if combination succeeded, false if no recipe exists
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool TryCombineItems(int32 SlotIndexA, int32 SlotIndexB);

	/**
	 * Restore inventory from saved data. Replaces all current slots.
	 * Broadcasts OnInventoryChanged after restoration.
	 * @param SavedSlots Array of saved inventory slots to restore
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RestoreSavedInventory(const TArray<FInventorySlot>& SavedSlots);

	// --- Delegates ---

	/** Broadcast when any slot changes (add, remove, modify). */
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChanged OnInventoryChanged;

	/** Broadcast when an inventory action fails (e.g., inventory full). */
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryActionFailed OnInventoryActionFailed;

	/** Broadcast when an item combination fails (no recipe exists). */
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnCombineFailed OnCombineFailed;

protected:
	/** Inventory slots array. Initialized to MaxSlots empty slots. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<FInventorySlot> Slots;

	/** Registry mapping ItemId to item data asset. Populated from Asset Manager at BeginPlay. */
	UPROPERTY()
	TMap<FName, TObjectPtr<UItemDataAsset>> ItemRegistry;

private:
	/** Load all UItemDataAsset instances from Asset Manager. */
	void LoadItemRegistry();

	/** Combine recipes: {ItemA, ItemB} -> ResultItem. Order-independent (both permutations checked). */
	TMap<TPair<FName, FName>, FName> CombineRecipes;

	/** Initialize combine recipes. Called in BeginPlay after registry load. */
	void InitCombineRecipes();
};
