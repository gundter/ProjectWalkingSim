// Copyright Null Lantern.

#include "Interaction/PickupActor.h"

#include "Inventory/ItemDataAsset.h"
#include "Tags/SereneTags.h"
#include "Core/SereneLogChannels.h"

APickupActor::APickupActor()
{
	InteractionText = NSLOCTEXT("Interaction", "PickUp", "Pick Up");
	InteractionTag = SereneTags::TAG_Interaction_Pickup;
}

void APickupActor::OnInteract_Implementation(AActor* Interactor)
{
	// Phase 2 will add actual inventory integration here.
	// For now, log the pickup event and optionally destroy.

	UE_LOG(LogSerene, Log, TEXT("APickupActor::OnInteract - Picked up %s x%d"),
		*ItemId.ToString(), Quantity);

	if (bDestroyOnPickup)
	{
		Destroy();
	}
}

void APickupActor::InitFromItemData(FName InItemId, int32 InQuantity, const UItemDataAsset* ItemData)
{
	ItemId = InItemId;
	Quantity = InQuantity;

	// Mesh loading and interaction text update added in Plan 03
	UE_LOG(LogSerene, Log, TEXT("APickupActor::InitFromItemData - Initialized with ItemId=%s, Quantity=%d"),
		*InItemId.ToString(), InQuantity);
}
