// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace SereneTags
{
	// Interaction types
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Interaction_Door);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Interaction_Pickup);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Interaction_Readable);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Interaction_Drawer);

	// Movement states
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Movement_Walking);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Movement_Sprinting);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Movement_Crouching);

	// Player states
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Player_Exhausted);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Player_Leaning);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Player_InTransition);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Player_InventoryOpen);

	// Item types
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item_Key);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item_Consumable);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item_Tool);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item_Puzzle);
}
