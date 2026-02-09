// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "InventoryTypes.generated.h"

/**
 * Item type classification for The Juniper Tree inventory system.
 * Used to determine item behavior and UI categorization.
 */
UENUM(BlueprintType)
enum class EItemType : uint8
{
	KeyItem,     // Keys, codes - critical path items
	Consumable,  // Future: healing, etc.
	Tool,        // Future: flashlight battery, etc.
	Puzzle       // Fuse, handle, etc.
};

/**
 * Runtime representation of a single inventory slot.
 * Stores the item identifier and current quantity.
 */
USTRUCT(BlueprintType)
struct PROJECTWALKINGSIM_API FInventorySlot
{
	GENERATED_BODY()

	/** Identifier matching UItemDataAsset::ItemId. NAME_None indicates empty slot. */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	FName ItemId = NAME_None;

	/** Current quantity of this item in the slot. */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 Quantity = 0;

	/** Returns true if this slot contains no item. */
	bool IsEmpty() const { return ItemId == NAME_None || Quantity <= 0; }
};

/** Broadcast when any inventory slot changes (add, remove, modify). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

/** Broadcast when an inventory action fails (e.g., inventory full). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryActionFailed, FName, ItemId, FText, Reason);
