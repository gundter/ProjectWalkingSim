// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HideableInterface.generated.h"

class UCameraComponent;
class UHidingSpotDataAsset;

UINTERFACE(MinimalAPI, Blueprintable)
class UHideable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Contract for actors that provide a hiding spot (closets, beds, lockers).
 * Implementers provide camera, data, occupancy, and discovery information.
 */
class PROJECTWALKINGSIM_API IHideable
{
	GENERATED_BODY()

public:
	// --- Core hiding lifecycle ---

	/** Whether the hiding actor can enter this spot right now. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hiding")
	bool CanHide(AActor* HidingActor) const;

	/** Called when an actor enters this hiding spot. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hiding")
	void OnEnterHiding(AActor* HidingActor);

	/** Called when an actor exits this hiding spot. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hiding")
	void OnExitHiding(AActor* HidingActor);

	// --- Spot information ---

	/** Returns the camera component used while hiding in this spot. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hiding")
	UCameraComponent* GetHidingCamera() const;

	/** Returns the data asset with per-type configuration for this spot. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hiding")
	UHidingSpotDataAsset* GetSpotData() const;

	/** Whether someone is already hiding in this spot. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hiding")
	bool IsOccupied() const;

	// --- Discovery (monster interaction) ---

	/** Mark this spot as discovered by the monster (player was found here). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hiding")
	void MarkDiscovered();

	/** Check if this spot was previously discovered by the monster. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hiding")
	bool WasDiscovered() const;
};
