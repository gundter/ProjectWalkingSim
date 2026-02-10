// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "HidingTypes.generated.h"

/**
 * Represents the current state of a player's hiding action.
 */
UENUM(BlueprintType)
enum class EHidingState : uint8
{
	Free     UMETA(DisplayName = "Free"),
	Entering UMETA(DisplayName = "Entering"),
	Hidden   UMETA(DisplayName = "Hidden"),
	Exiting  UMETA(DisplayName = "Exiting")
};

/** Broadcast when the player's hiding state changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHidingStateChanged, EHidingState, NewState);
