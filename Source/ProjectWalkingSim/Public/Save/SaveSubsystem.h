// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Save/SaveTypes.h"
#include "SaveSubsystem.generated.h"

class USereneSaveGame;

/**
 * Game instance subsystem that owns save/load orchestration for The Juniper Tree.
 *
 * Persists across level transitions (lives on USereneGameInstance). Provides:
 * - Save to one of 3 named slots with viewport screenshot capture
 * - Load from slot via level restart + pending data application
 * - Quick-load latest save
 * - Runtime tracking of destroyed level-placed pickups
 *
 * Save flow:
 *  1. SaveToSlot -> request screenshot (async, end-of-frame)
 *  2. OnScreenshotCaptured -> compress JPEG, gather world state, async write to disk
 *
 * Load flow:
 *  1. LoadFromSlot -> store PendingSaveData, OpenLevel (full world reset)
 *  2. GameMode calls ApplyPendingSaveData after actors exist
 *  3. Doors restored, pickups destroyed, player repositioned, inventory repopulated
 */
UCLASS()
class PROJECTWALKINGSIM_API USaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Maximum number of save slots. */
	static constexpr int32 MaxSlots = 3;

	// --- Save API ---

	/**
	 * Save the current game state to a slot.
	 * Captures a viewport screenshot (async), gathers world + player state,
	 * then writes via AsyncSaveGameToSlot.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveToSlot(int32 SlotIndex);

	/**
	 * Load a saved game from slot. Stores save data as pending, then reloads the level.
	 * After level reload, GameMode must call ApplyPendingSaveData().
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void LoadFromSlot(int32 SlotIndex);

	/**
	 * Find the most recent save across all slots and load it.
	 * Does nothing if no saves exist.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void LoadLatestSave();

	/** Returns true if any of the 3 slots has a save file. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Save")
	bool HasAnySave() const;

	/**
	 * Get metadata (timestamp + screenshot) for a specific slot.
	 * Returns empty info if slot does not exist.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Save")
	FSaveSlotInfo GetSlotInfo(int32 SlotIndex) const;

	/** Delete a save slot entirely. */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void DeleteSlot(int32 SlotIndex);

	/** Check whether a save exists in the given slot. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Save")
	bool DoesSaveExist(int32 SlotIndex) const;

	/** Returns the index of the most recent save, or -1 if no saves exist. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Save")
	int32 GetLatestSlotIndex() const;

	// --- State Management ---

	/**
	 * Apply pending save data to the world after a level reload.
	 * Called by GameMode once all actors exist. Restores doors, destroys
	 * picked-up items, repositions the player, and repopulates inventory.
	 * Clears PendingSaveData after application.
	 */
	void ApplyPendingSaveData(UWorld* World);

	/** Whether a load is pending (set before OpenLevel, read after). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Save")
	bool IsPendingLoad() const { return PendingSaveData != nullptr; }

	/**
	 * Get the pending save data for direct access (e.g., character restoring its own inventory).
	 * Returns nullptr if no load is pending.
	 */
	const USereneSaveGame* GetPendingSaveData() const { return PendingSaveData; }

	/**
	 * Track a destroyed level-placed pickup at runtime.
	 * Called by PickupActor when a level-placed pickup is picked up and destroyed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void TrackDestroyedPickup(FName PickupId);

	/** Clear the destroyed pickup tracker (called after applying save data). */
	void ClearDestroyedPickupTracker();

private:
	/** Generate the platform slot name for a given index. */
	FString GetSlotName(int32 SlotIndex) const;

	/** Gather world state (doors, drawers) into the save game object. */
	void GatherWorldState(USereneSaveGame* SaveGame, UWorld* World);

	/** Gather player state (location, rotation, inventory) into the save game object. */
	void GatherPlayerState(USereneSaveGame* SaveGame, UWorld* World);

	// --- Screenshot Capture ---

	/** Callback for UGameViewportClient::OnScreenshotCaptured. */
	void OnScreenshotCaptured(int32 Width, int32 Height, const TArray<FColor>& Bitmap);

	/** Delegate handle for unbinding the screenshot callback. */
	FDelegateHandle ScreenshotDelegateHandle;

	/** Save object being constructed during a save flow (between screenshot request and write). */
	UPROPERTY()
	TObjectPtr<USereneSaveGame> PendingSaveObject;

	/** Slot index for the in-progress save (used in screenshot callback). */
	int32 PendingSaveSlotIndex = -1;

	// --- Pending Load Data ---

	/** Save data to apply after level reload. Prevents GC via UPROPERTY. */
	UPROPERTY()
	TObjectPtr<USereneSaveGame> PendingSaveData;

	/** Runtime tracking of destroyed level-placed pickups. */
	TSet<FName> DestroyedPickupTracker;
};
