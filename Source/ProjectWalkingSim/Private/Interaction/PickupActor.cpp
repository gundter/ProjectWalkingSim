// Copyright Null Lantern.

#include "Interaction/PickupActor.h"

#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemDataAsset.h"
#include "Save/SaveSubsystem.h"
#include "Tags/SereneTags.h"
#include "Core/SereneLogChannels.h"
#include "Engine/AssetManager.h"

APickupActor::APickupActor()
{
	InteractionText = NSLOCTEXT("Interaction", "PickUp", "Pick Up");
	InteractionTag = SereneTags::TAG_Interaction_Pickup;
}

void APickupActor::BeginPlay()
{
	Super::BeginPlay();

	// Look up item data to set proper interaction text with item name
	if (ItemId != NAME_None)
	{
		UAssetManager& AssetManager = UAssetManager::Get();

		// Get all registered Item assets and find the one with matching ItemId
		TArray<FPrimaryAssetId> AssetList;
		AssetManager.GetPrimaryAssetIdList(FPrimaryAssetType("Item"), AssetList);

		for (const FPrimaryAssetId& AssetId : AssetList)
		{
			UObject* LoadedObject = AssetManager.GetPrimaryAssetObject(AssetId);
			if (!LoadedObject)
			{
				FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(AssetId);
				if (!AssetPath.IsNull())
				{
					// Synchronous load -- acceptable for <30 items. Consider LoadPrimaryAssetsAsync if item count grows.
				LoadedObject = AssetPath.TryLoad();
				}
			}

			if (const UItemDataAsset* ItemData = Cast<UItemDataAsset>(LoadedObject))
			{
				if (ItemData->ItemId == ItemId)
				{
					if (!ItemData->DisplayName.IsEmpty())
					{
						InteractionText = FText::Format(
							NSLOCTEXT("Interaction", "PickUpItem", "Pick Up {0}"),
							ItemData->DisplayName);
					}
					return; // Found our item, done
				}
			}
		}

		UE_LOG(LogSerene, Warning, TEXT("APickupActor::BeginPlay - Item '%s' not found in Asset Manager"), *ItemId.ToString());
	}
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
			// Notify save subsystem before destruction so reload can re-destroy this pickup
			if (UGameInstance* GI = GetGameInstance())
			{
				if (USaveSubsystem* SaveSys = GI->GetSubsystem<USaveSubsystem>())
				{
					SaveSys->TrackDestroyedPickup(GetFName());
				}
			}
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
	if (ItemData && !ItemData->WorldMesh.IsNull())
	{
		// Synchronous load on discard -- acceptable for player-initiated action with small item set.
		UStaticMesh* Mesh = ItemData->WorldMesh.LoadSynchronous();
		if (Mesh && MeshComponent)
		{
			MeshComponent->SetStaticMesh(Mesh);
		}
	}
	else if (ItemData)
	{
		UE_LOG(LogSerene, Warning, TEXT("APickupActor::InitFromItemData - WorldMesh is null for item '%s'. Discarded item will be invisible."), *InItemId.ToString());
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

// ---------------------------------------------------------------------------
// ISaveable
// ---------------------------------------------------------------------------

FName APickupActor::GetSaveId_Implementation() const
{
	return GetFName();
}

void APickupActor::WriteSaveData_Implementation(USereneSaveGame* SaveGame)
{
	// Pickup destruction is tracked by SaveSubsystem::DestroyedPickupTracker,
	// not by individual actor write. No-op.
}

void APickupActor::ReadSaveData_Implementation(USereneSaveGame* SaveGame)
{
	// Pickup destruction is handled by SaveSubsystem::ApplyPendingSaveData
	// which destroys pickups by FName. No-op.
}
