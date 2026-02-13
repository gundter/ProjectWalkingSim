// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "SaveTypes.generated.h"

/**
 * Serializable state snapshot of a single door actor.
 * Stored as flat array in USereneSaveGame for level-placed doors.
 */
USTRUCT()
struct FSavedDoorState
{
	GENERATED_BODY()

	/** Actor FName in the level -- used to match on load. */
	UPROPERTY()
	FName DoorId;

	UPROPERTY()
	bool bIsOpen = false;

	UPROPERTY()
	bool bIsLocked = false;

	UPROPERTY()
	float CurrentAngle = 0.0f;

	UPROPERTY()
	float OpenDirection = 1.0f;
};

/**
 * Serializable state snapshot of a single drawer actor.
 * Stored as flat array in USereneSaveGame for level-placed drawers.
 */
USTRUCT()
struct FSavedDrawerState
{
	GENERATED_BODY()

	/** Actor FName in the level -- used to match on load. */
	UPROPERTY()
	FName DrawerId;

	UPROPERTY()
	bool bIsOpen = false;

	UPROPERTY()
	float CurrentSlide = 0.0f;
};

/**
 * Metadata for a single save slot.
 * Displayed in the save/load UI: screenshot thumbnail + timestamp.
 */
USTRUCT()
struct FSaveSlotInfo
{
	GENERATED_BODY()

	/** When this save was created. */
	UPROPERTY()
	FDateTime Timestamp;

	/** Compressed JPEG bytes of the viewport screenshot at save time. */
	UPROPERTY()
	TArray<uint8> ScreenshotData;

	/** Width of the captured screenshot in pixels. */
	UPROPERTY()
	int32 ScreenshotWidth = 0;

	/** Height of the captured screenshot in pixels. */
	UPROPERTY()
	int32 ScreenshotHeight = 0;
};
