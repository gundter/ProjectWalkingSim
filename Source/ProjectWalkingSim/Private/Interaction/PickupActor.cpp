// Copyright Null Lantern.

#include "Interaction/PickupActor.h"

#include "Tags/SereneTags.h"
#include "Core/SereneLogChannels.h"

APickupActor::APickupActor()
{
	InteractionText = NSLOCTEXT("Interaction", "PickUp", "Pick Up");
	InteractionTag = SereneTags::TAG_Interaction_Pickup;
}

void APickupActor::OnInteract_Implementation(AActor* Interactor)
{
	// Phase 2 will add actual inventory integration here.
	// For now, log the pickup event and optionally destroy.

	UE_LOG(LogSerene, Log, TEXT("APickupActor::OnInteract - Picked up %s x%d"),
		*ItemId.ToString(), Quantity);

	if (bDestroyOnPickup)
	{
		Destroy();
	}
}
