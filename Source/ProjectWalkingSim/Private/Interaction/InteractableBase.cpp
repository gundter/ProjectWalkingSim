// Copyright Null Lantern.

#include "Interaction/InteractableBase.h"

AInteractableBase::AInteractableBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root mesh for visual representation
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);

	// Default interaction text
	InteractionText = NSLOCTEXT("Interaction", "Default", "Interact");
}

FText AInteractableBase::GetInteractionText_Implementation() const
{
	return InteractionText;
}

bool AInteractableBase::CanInteract_Implementation(AActor* Interactor) const
{
	return bCanBeInteracted;
}

void AInteractableBase::OnInteract_Implementation(AActor* Interactor)
{
	// Empty default. Subclasses override with specific behavior.
}

void AInteractableBase::OnFocusBegin_Implementation(AActor* Interactor)
{
	// Empty default. Future: highlight effect, outline shader, etc.
}

void AInteractableBase::OnFocusEnd_Implementation(AActor* Interactor)
{
	// Empty default. Future: remove highlight effect.
}
