// Copyright Null Lantern.

#include "Interaction/PickupActor.h"

#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemDataAsset.h"
#include "Tags/SereneTags.h"
#include "Core/SereneLogChannels.h"

APickupActor::APickupActor()
{
	InteractionText = NSLOCTEXT("Interaction", "PickUp", "Pick Up");
	InteractionTag = SereneTags::TAG_Interaction_Pickup;
}

bool APickupActor::CanInteract_Implementation(AActor* Interactor) const
{
	// Check base class first
	if (!Super::CanInteract_Implementation(Interactor))
	{
		return false;
	}

	// Find inventory component on interactor
	UInventoryComponent* Inventory = Interactor ? Interactor->FindComponentByClass<UInventoryComponent>() : nullptr;

	if (!Inventory)
	{
		// No inventory component - graceful fallback (Phase 1 behavior)
		bInventoryFullOnLastCheck = false;
		return true;
	}

	// Check if inventory is full
	if (Inventory->IsFull())
	{
		// Check if item is stackable AND already exists in inventory (can still add to existing stack)
		const UItemDataAsset* Data = Inventory->GetItemData(ItemId);
		if (Data && Data->bIsStackable && Inventory->HasItem(ItemId))
		{
			// Can stack onto existing item
			bInventoryFullOnLastCheck = false;
			return true;
		}

		// Inventory truly full - still allow interaction for feedback, but show different text
		bInventoryFullOnLastCheck = true;
		return true;
	}

	bInventoryFullOnLastCheck = false;
	return true;
}

FText APickupActor::GetInteractionText_Implementation() const
{
	if (bInventoryFullOnLastCheck)
	{
		return NSLOCTEXT("Interaction", "InventoryFull", "Inventory Full");
	}

	return Super::GetInteractionText_Implementation();
}

void APickupActor::OnInteract_Implementation(AActor* Interactor)
{
	// Find inventory component on interactor
	UInventoryComponent* Inventory = Interactor ? Interactor->FindComponentByClass<UInventoryComponent>() : nullptr;

	if (!Inventory)
	{
		UE_LOG(LogSerene, Warning, TEXT("APickupActor::OnInteract - Interactor has no InventoryComponent, cannot pick up %s"),
			*ItemId.ToString());
		return;
	}

	// Try to add item to inventory
	if (Inventory->TryAddItem(ItemId, Quantity))
	{
		UE_LOG(LogSerene, Log, TEXT("APickupActor::OnInteract - Picked up %s x%d"),
			*ItemId.ToString(), Quantity);

		if (bDestroyOnPickup)
		{
			Destroy();
		}
	}
	else
	{
		UE_LOG(LogSerene, Log, TEXT("APickupActor::OnInteract - Inventory full, cannot pick up %s"),
			*ItemId.ToString());
		// Do NOT destroy - item stays in world
	}
}

void APickupActor::InitFromItemData(FName InItemId, int32 InQuantity, const UItemDataAsset* ItemData)
{
	ItemId = InItemId;
	Quantity = InQuantity;

	// Load and set world mesh if available
	if (ItemData && ItemData->WorldMesh.IsValid())
	{
		UStaticMesh* Mesh = ItemData->WorldMesh.LoadSynchronous();
		if (Mesh && MeshComponent)
		{
			MeshComponent->SetStaticMesh(Mesh);
		}
	}

	// Set interaction text from item display name or fallback
	if (ItemData && !ItemData->DisplayName.IsEmpty())
	{
		InteractionText = FText::Format(
			NSLOCTEXT("Interaction", "PickUpItem", "Pick Up {0}"),
			ItemData->DisplayName);
	}
	else
	{
		InteractionText = NSLOCTEXT("Interaction", "PickUp", "Pick Up");
	}

	UE_LOG(LogSerene, Log, TEXT("APickupActor::InitFromItemData - Initialized with ItemId=%s, Quantity=%d"),
		*InItemId.ToString(), InQuantity);
}
