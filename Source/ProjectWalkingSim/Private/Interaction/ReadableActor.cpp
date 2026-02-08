// Copyright Null Lantern.

#include "Interaction/ReadableActor.h"

#include "Tags/SereneTags.h"
#include "Core/SereneLogChannels.h"

AReadableActor::AReadableActor()
{
	InteractionText = NSLOCTEXT("Interaction", "Read", "Read");
	InteractionTag = SereneTags::TAG_Interaction_Readable;
}

void AReadableActor::OnInteract_Implementation(AActor* Interactor)
{
	// Full text display UI deferred to a later phase.
	// For now, log the title to confirm the interaction fires.

	UE_LOG(LogSerene, Log, TEXT("AReadableActor::OnInteract - Reading: %s"),
		*ReadableTitle.ToString());
}
