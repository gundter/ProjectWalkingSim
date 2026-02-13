// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SaveableInterface.generated.h"

class USereneSaveGame;

UINTERFACE(MinimalAPI, Blueprintable)
class USaveable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Contract for actors that persist state across save/load cycles.
 *
 * Actors implementing this interface write their state into and read
 * it back from USereneSaveGame. The save subsystem calls WriteSaveData
 * during save and ReadSaveData during load for every ISaveable actor
 * found in the world.
 */
class PROJECTWALKINGSIM_API ISaveable
{
	GENERATED_BODY()

public:
	/** Return a unique, stable identifier for this actor instance. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
	FName GetSaveId() const;

	/** Write this actor's state into the save game object. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
	void WriteSaveData(USereneSaveGame* SaveGame);

	/** Restore this actor's state from saved data. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
	void ReadSaveData(USereneSaveGame* SaveGame);
};
