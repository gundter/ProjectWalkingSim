// Copyright Null Lantern.

#include "Inventory/InventoryComponent.h"

#include "Inventory/ItemDataAsset.h"
#include "Interaction/PickupActor.h"
#include "Core/SereneLogChannels.h"
#include "Engine/AssetManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Initialize slots array to MaxSlots empty slots
	Slots.SetNum(MaxSlots);
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	LoadItemRegistry();
	InitCombineRecipes();
}

void UInventoryComponent::LoadItemRegistry()
{
	UAssetManager& AssetManager = UAssetManager::Get();

	TArray<FPrimaryAssetId> AssetList;
	AssetManager.GetPrimaryAssetIdList(FPrimaryAssetType("Item"), AssetList);

	for (const FPrimaryAssetId& AssetId : AssetList)
	{
		// Try to get already-loaded asset first
		UObject* LoadedObject = AssetManager.GetPrimaryAssetObject(AssetId);

		// If not loaded, load synchronously via path resolution (acceptable for small item count)
		if (!LoadedObject)
		{
			FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(AssetId);
			if (!AssetPath.IsNull())
			{
				LoadedObject = AssetPath.TryLoad();
			}
		}

		if (UItemDataAsset* ItemData = Cast<UItemDataAsset>(LoadedObject))
		{
			ItemRegistry.Add(ItemData->ItemId, ItemData);
		}
	}

	UE_LOG(LogSerene, Log, TEXT("UInventoryComponent::LoadItemRegistry - Registered %d items"), ItemRegistry.Num());
}

bool UInventoryComponent::TryAddItem(FName ItemId, int32 Quantity)
{
	if (ItemId == NAME_None || Quantity <= 0)
	{
		UE_LOG(LogSerene, Warning, TEXT("UInventoryComponent::TryAddItem - Invalid ItemId or Quantity"));
		return false;
	}

	// Check if item exists in registry
	const UItemDataAsset* ItemData = GetItemData(ItemId);
	if (!ItemData)
	{
		UE_LOG(LogSerene, Warning, TEXT("UInventoryComponent::TryAddItem - Item '%s' not found in registry"), *ItemId.ToString());
		return false;
	}

	int32 RemainingQuantity = Quantity;

	// If stackable, try to add to existing stacks first
	if (ItemData->bIsStackable)
	{
		for (FInventorySlot& Slot : Slots)
		{
			if (Slot.ItemId == ItemId && Slot.Quantity < ItemData->MaxStackSize)
			{
				const int32 SpaceInStack = ItemData->MaxStackSize - Slot.Quantity;
				const int32 ToAdd = FMath::Min(RemainingQuantity, SpaceInStack);
				Slot.Quantity += ToAdd;
				RemainingQuantity -= ToAdd;

				if (RemainingQuantity <= 0)
				{
					break;
				}
			}
		}
	}

	// Add remaining to empty slots
	while (RemainingQuantity > 0)
	{
		const int32 EmptySlot = FindFirstEmptySlot();
		if (EmptySlot < 0)
		{
			// Inventory full
			UE_LOG(LogSerene, Log, TEXT("UInventoryComponent::TryAddItem - Inventory full, cannot add %s (remaining: %d)"),
				*ItemId.ToString(), RemainingQuantity);

			OnInventoryActionFailed.Broadcast(ItemId, NSLOCTEXT("Inventory", "Full", "Inventory is full"));
			return false;
		}

		const int32 ToAdd = ItemData->bIsStackable
			? FMath::Min(RemainingQuantity, ItemData->MaxStackSize)
			: 1;

		Slots[EmptySlot].ItemId = ItemId;
		Slots[EmptySlot].Quantity = ToAdd;
		RemainingQuantity -= ToAdd;
	}

	UE_LOG(LogSerene, Log, TEXT("UInventoryComponent::TryAddItem - Added %s x%d to inventory"), *ItemId.ToString(), Quantity);
	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::RemoveItem(int32 SlotIndex, int32 Quantity)
{
	if (!Slots.IsValidIndex(SlotIndex))
	{
		UE_LOG(LogSerene, Warning, TEXT("UInventoryComponent::RemoveItem - Invalid slot index %d"), SlotIndex);
		return false;
	}

	FInventorySlot& Slot = Slots[SlotIndex];
	if (Slot.IsEmpty())
	{
		UE_LOG(LogSerene, Warning, TEXT("UInventoryComponent::RemoveItem - Slot %d is empty"), SlotIndex);
		return false;
	}

	if (Slot.Quantity < Quantity)
	{
		UE_LOG(LogSerene, Warning, TEXT("UInventoryComponent::RemoveItem - Slot %d has %d, requested %d"),
			SlotIndex, Slot.Quantity, Quantity);
		return false;
	}

	Slot.Quantity -= Quantity;
	if (Slot.Quantity <= 0)
	{
		Slot.ItemId = NAME_None;
		Slot.Quantity = 0;
	}

	UE_LOG(LogSerene, Log, TEXT("UInventoryComponent::RemoveItem - Removed %d from slot %d"), Quantity, SlotIndex);
	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::RemoveItemByName(FName ItemId, int32 Quantity)
{
	const int32 SlotIndex = FindSlotWithItem(ItemId);
	if (SlotIndex < 0)
	{
		UE_LOG(LogSerene, Warning, TEXT("UInventoryComponent::RemoveItemByName - Item '%s' not found"), *ItemId.ToString());
		return false;
	}

	return RemoveItem(SlotIndex, Quantity);
}

bool UInventoryComponent::HasItem(FName ItemId) const
{
	return FindSlotWithItem(ItemId) >= 0;
}

int32 UInventoryComponent::GetItemCount(FName ItemId) const
{
	int32 TotalCount = 0;
	for (const FInventorySlot& Slot : Slots)
	{
		if (Slot.ItemId == ItemId)
		{
			TotalCount += Slot.Quantity;
		}
	}
	return TotalCount;
}

bool UInventoryComponent::IsFull() const
{
	return FindFirstEmptySlot() < 0;
}

const UItemDataAsset* UInventoryComponent::GetItemData(FName ItemId) const
{
	if (const TObjectPtr<UItemDataAsset>* Found = ItemRegistry.Find(ItemId))
	{
		return *Found;
	}
	return nullptr;
}

void UInventoryComponent::DiscardItem(int32 SlotIndex)
{
	if (!Slots.IsValidIndex(SlotIndex) || Slots[SlotIndex].IsEmpty())
	{
		UE_LOG(LogSerene, Warning, TEXT("UInventoryComponent::DiscardItem - Invalid or empty slot %d"), SlotIndex);
		return;
	}

	const FInventorySlot& Slot = Slots[SlotIndex];
	const UItemDataAsset* ItemData = GetItemData(Slot.ItemId);

	// Warn if discarding a key item
	if (ItemData && ItemData->bIsKeyItem)
	{
		UE_LOG(LogSerene, Warning, TEXT("UInventoryComponent::DiscardItem - Discarding key item '%s'"), *Slot.ItemId.ToString());
	}

	// Get owner's viewpoint for spawn location
	FVector EyeLocation = FVector::ZeroVector;
	FRotator EyeRotation = FRotator::ZeroRotator;

	AActor* Owner = GetOwner();
	if (APawn* Pawn = Cast<APawn>(Owner))
	{
		if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
		{
			PC->GetPlayerViewPoint(EyeLocation, EyeRotation);
		}
	}

	// Spawn 80cm in front of eye position
	const FVector SpawnLocation = EyeLocation + EyeRotation.Vector() * 80.0f;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APickupActor* DroppedItem = GetWorld()->SpawnActor<APickupActor>(
		APickupActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);

	if (DroppedItem)
	{
		// Initialize the spawned pickup with item data
		DroppedItem->InitFromItemData(Slot.ItemId, Slot.Quantity, ItemData);

		// Enable physics and apply impulse
		if (UPrimitiveComponent* PrimComp = DroppedItem->FindComponentByClass<UPrimitiveComponent>())
		{
			PrimComp->SetSimulatePhysics(true);
			const FVector Impulse = EyeRotation.Vector() * 150.0f + FVector(0.0f, 0.0f, 50.0f);
			PrimComp->AddImpulse(Impulse);
		}

		UE_LOG(LogSerene, Log, TEXT("UInventoryComponent::DiscardItem - Spawned %s x%d at (%s)"),
			*Slot.ItemId.ToString(), Slot.Quantity, *SpawnLocation.ToString());
	}

	// Remove from inventory (remove all quantity in the slot)
	RemoveItem(SlotIndex, Slot.Quantity);
}

int32 UInventoryComponent::FindFirstEmptySlot() const
{
	for (int32 i = 0; i < Slots.Num(); i++)
	{
		if (Slots[i].IsEmpty())
		{
			return i;
		}
	}
	return -1;
}

int32 UInventoryComponent::FindSlotWithItem(FName ItemId) const
{
	for (int32 i = 0; i < Slots.Num(); i++)
	{
		if (Slots[i].ItemId == ItemId)
		{
			return i;
		}
	}
	return -1;
}

void UInventoryComponent::InitCombineRecipes()
{
	// Add recipes here as the demo expands. Format: {ItemA, ItemB} -> Result
	// Example (commented out until actual items exist):
	// CombineRecipes.Add(TPair<FName,FName>(FName("Fuse"), FName("FuseBox")), FName("PoweredFuseBox"));

	UE_LOG(LogSerene, Log, TEXT("UInventoryComponent::InitCombineRecipes - %d recipes loaded."), CombineRecipes.Num());
}

bool UInventoryComponent::TryCombineItems(int32 SlotIndexA, int32 SlotIndexB)
{
	// Validate slot indices
	if (!Slots.IsValidIndex(SlotIndexA) || !Slots.IsValidIndex(SlotIndexB))
	{
		UE_LOG(LogSerene, Warning, TEXT("UInventoryComponent::TryCombineItems - Invalid slot index (A=%d, B=%d)"), SlotIndexA, SlotIndexB);
		return false;
	}

	// Ensure slots are not the same
	if (SlotIndexA == SlotIndexB)
	{
		UE_LOG(LogSerene, Warning, TEXT("UInventoryComponent::TryCombineItems - Cannot combine slot with itself (slot %d)"), SlotIndexA);
		return false;
	}

	// Ensure neither slot is empty
	if (Slots[SlotIndexA].IsEmpty() || Slots[SlotIndexB].IsEmpty())
	{
		UE_LOG(LogSerene, Warning, TEXT("UInventoryComponent::TryCombineItems - Cannot combine empty slots (A=%d empty=%d, B=%d empty=%d)"),
			SlotIndexA, Slots[SlotIndexA].IsEmpty(), SlotIndexB, Slots[SlotIndexB].IsEmpty());
		return false;
	}

	const FName ItemIdA = Slots[SlotIndexA].ItemId;
	const FName ItemIdB = Slots[SlotIndexB].ItemId;

	// Check recipe map with both orderings (order-independent)
	FName* ResultItem = CombineRecipes.Find(TPair<FName, FName>(ItemIdA, ItemIdB));
	if (!ResultItem)
	{
		ResultItem = CombineRecipes.Find(TPair<FName, FName>(ItemIdB, ItemIdA));
	}

	if (ResultItem)
	{
		// Recipe found - perform combination
		UE_LOG(LogSerene, Log, TEXT("UInventoryComponent::TryCombineItems - Combining %s + %s = %s"),
			*ItemIdA.ToString(), *ItemIdB.ToString(), *ResultItem->ToString());

		// Remove 1 quantity from both source slots
		RemoveItem(SlotIndexA, 1);
		RemoveItem(SlotIndexB, 1);

		// Add the result item
		TryAddItem(*ResultItem, 1);

		// Note: RemoveItem and TryAddItem already broadcast OnInventoryChanged
		return true;
	}

	// No recipe found
	UE_LOG(LogSerene, Log, TEXT("UInventoryComponent::TryCombineItems - No recipe for %s + %s"),
		*ItemIdA.ToString(), *ItemIdB.ToString());

	OnCombineFailed.Broadcast(NSLOCTEXT("Inventory", "CombineFailed", "These items cannot be combined"));
	return false;
}
