// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Inventory/InventoryTypes.h"
#include "ItemDataAsset.generated.h"

/**
 * Data asset defining static properties for an inventory item.
 *
 * Each item type in The Juniper Tree has a corresponding UItemDataAsset instance
 * created in the Content Browser under Content/Data/Items/. The ItemId must match
 * the ItemId on APickupActor instances in the world.
 *
 * Uses TSoftObjectPtr for Icon and WorldMesh to avoid loading all item visuals
 * at startup. Call LoadSynchronous() when the asset is actually needed.
 *
 * The Asset Manager discovers these via GetPrimaryAssetId() which returns
 * FPrimaryAssetId("Item", GetFName()). Configure the "Item" primary asset type
 * in Project Settings > Game > Asset Manager.
 */
UCLASS(BlueprintType)
class PROJECTWALKINGSIM_API UItemDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Returns a unique identifier for Asset Manager registration. */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	// --- Item Identity ---

	/** Unique identifier matching APickupActor::ItemId. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FName ItemId;

	/** Player-facing name displayed in UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	/** Player-facing description shown in item tooltip. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText Description;

	// --- Item Behavior ---

	/** Classification affecting item behavior and UI grouping. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	EItemType ItemType = EItemType::KeyItem;

	/** If true, multiple pickups consolidate into a single slot. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	bool bIsStackable = false;

	/** Maximum quantity per slot. Only relevant when bIsStackable is true. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item",
		meta = (EditCondition = "bIsStackable", ClampMin = "1", ClampMax = "99"))
	int32 MaxStackSize = 1;

	/** If true, this item is critical for progression and warns on discard. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	bool bIsKeyItem = false;

	/** GameplayTag for item-specific logic (e.g., Item.Key.FrontDoor). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FGameplayTag ItemTag;

	// --- Item Visuals ---

	/** Icon displayed in inventory slot. Soft reference to avoid eager loading. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Visuals")
	TSoftObjectPtr<UTexture2D> Icon;

	/** Mesh spawned when item is discarded into world. Soft reference to avoid eager loading. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Visuals")
	TSoftObjectPtr<UStaticMesh> WorldMesh;
};
