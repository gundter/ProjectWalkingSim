// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Save/SaveTypes.h"
#include "Inventory/InventoryTypes.h"
#include "SereneSaveGame.generated.h"

/**
 * Save game container for The Juniper Tree.
 *
 * Pure data class -- all fields are UPROPERTY() for automatic USaveGame
 * serialization via UGameplayStatics::SaveGameToSlot / LoadGameFromSlot.
 *
 * Uses flat struct arrays (not per-actor binary serialization) because the
 * project has a known, limited set of saveable state types and a single level.
 */
UCLASS()
class PROJECTWALKINGSIM_API USereneSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	USereneSaveGame();

	// --- Versioning ---

	/** Incremented on schema changes; enables future migration logic. */
	UPROPERTY()
	int32 SaveVersion = 1;

	// --- Slot Metadata ---

	/** Timestamp + screenshot thumbnail for save slot UI. */
	UPROPERTY()
	FSaveSlotInfo SlotInfo;

	// --- Player State ---

	UPROPERTY()
	FVector PlayerLocation;

	UPROPERTY()
	FRotator PlayerRotation;

	/** Snapshot of the player's 8-slot inventory at save time. */
	UPROPERTY()
	TArray<FInventorySlot> InventorySlots;

	// --- World State ---

	/** State of every level-placed door that was modified from its default. */
	UPROPERTY()
	TArray<FSavedDoorState> DoorStates;

	/** State of every level-placed drawer that was modified from its default. */
	UPROPERTY()
	TArray<FSavedDrawerState> DrawerStates;

	/** FNames of level-placed pickups that were picked up (destroyed). */
	UPROPERTY()
	TArray<FName> DestroyedPickupIds;
};
