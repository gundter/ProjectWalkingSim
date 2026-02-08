// Copyright Null Lantern.

#include "Tags/SereneTags.h"

namespace SereneTags
{
	// Interaction types
	UE_DEFINE_GAMEPLAY_TAG(TAG_Interaction_Door, "Interaction.Door");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Interaction_Pickup, "Interaction.Pickup");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Interaction_Readable, "Interaction.Readable");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Interaction_Drawer, "Interaction.Drawer");

	// Movement states
	UE_DEFINE_GAMEPLAY_TAG(TAG_Movement_Walking, "Movement.Walking");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Movement_Sprinting, "Movement.Sprinting");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Movement_Crouching, "Movement.Crouching");

	// Player states
	UE_DEFINE_GAMEPLAY_TAG(TAG_Player_Exhausted, "Player.Exhausted");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Player_Leaning, "Player.Leaning");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Player_InTransition, "Player.InTransition");
}
