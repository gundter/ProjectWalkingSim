// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractableBase.h"
#include "PickupActor.generated.h"

class UItemDataAsset;

/**
 * Generic pickup actor for items in The Juniper Tree.
 *
 * Integrates with the inventory system via UInventoryComponent. On interaction,
 * attempts to add the item to the player's inventory. Only destroys on successful
 * pickup. Shows "Inventory Full" when the player cannot pick up the item.
 *
 * Designers set ItemId and Quantity per instance. The mesh represents the
 * physical item in the world.
 */
UCLASS()
class PROJECTWALKINGSIM_API APickupActor : public AInteractableBase
{
	GENERATED_BODY()

public:
	APickupActor();

	/**
	 * Initialize this pickup from item data. Used when spawning discarded items.
	 * Sets ItemId, Quantity, loads WorldMesh if available, and updates InteractionText.
	 * @param InItemId Item identifier
	 * @param InQuantity Item quantity
	 * @param ItemData Item data asset (may be null if item not in registry)
	 */
	void InitFromItemData(FName InItemId, int32 InQuantity, const UItemDataAsset* ItemData);

protected:
	virtual void BeginPlay() override;
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual FText GetInteractionText_Implementation() const override;

	/** Identifier for the inventory system. Matched to item data in Phase 2. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	FName ItemId;

	/** Number of this item picked up. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	int32 Quantity = 1;

	/** If true, the actor is destroyed after successful pickup. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	bool bDestroyOnPickup = true;

private:
	/** Cached state from CanInteract for GetInteractionText. Mutable because CanInteract is const. */
	mutable bool bInventoryFullOnLastCheck = false;
};
