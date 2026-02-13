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

	// Hiding system
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Player_Hiding);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Interaction_HidingSpot);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_HidingSpot_Locker);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_HidingSpot_Closet);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_HidingSpot_UnderBed);

	// AI alert levels
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Alert_Patrol);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Alert_Suspicious);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Alert_Alert);

	// AI stimulus types
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Stimulus_Sight);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Stimulus_Hearing);

	// AI behavior states
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Behavior_Patrol);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Behavior_Investigating);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Behavior_Chasing);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Behavior_Searching);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Behavior_GrabAttack);

	// AI spawn
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Spawn_Zone);

	// Narrative
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Interaction_Inspectable);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Narrative_Trigger);
}
