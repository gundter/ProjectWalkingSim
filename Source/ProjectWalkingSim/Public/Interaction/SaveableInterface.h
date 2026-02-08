// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SaveableInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class USaveable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Contract for actors that persist state across save/load cycles.
 * Stub interface -- fully implemented in Phase 7: Save System.
 */
class PROJECTWALKINGSIM_API ISaveable
{
	GENERATED_BODY()

public:
	/** Return a unique identifier for this saveable instance. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
	FString GetSaveId() const;

	/** Serialize this actor's state for saving. Parameters will be expanded in Phase 7. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
	void WriteSaveData();

	/** Restore this actor's state from saved data. Parameters will be expanded in Phase 7. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
	void ReadSaveData();
};
