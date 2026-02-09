// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractableBase.h"
#include "PickupActor.generated.h"

class UItemDataAsset;

/**
 * Generic pickup actor for items in The Juniper Tree.
 *
 * Placeholder for Phase 2 inventory integration. Currently logs the pickup
 * and optionally destroys itself. When the inventory system exists (Phase 2),
 * OnInteract will add the item to the player's inventory before destroying.
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
	 * Minimal version: sets ItemId and Quantity. Plan 03 extends this with mesh loading.
	 * @param InItemId Item identifier
	 * @param InQuantity Item quantity
	 * @param ItemData Item data asset (may be null if item not in registry)
	 */
	void InitFromItemData(FName InItemId, int32 InQuantity, const UItemDataAsset* ItemData);

protected:
	virtual void OnInteract_Implementation(AActor* Interactor) override;

	/** Identifier for the inventory system. Matched to item data in Phase 2. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	FName ItemId;

	/** Number of this item picked up. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	int32 Quantity = 1;

	/** If true, the actor is destroyed after successful pickup. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	bool bDestroyOnPickup = true;
};
