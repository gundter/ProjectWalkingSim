// Copyright Null Lantern.

#include "Interaction/DoorActor.h"

#include "Inventory/InventoryComponent.h"
#include "Tags/SereneTags.h"
#include "Core/SereneLogChannels.h"

ADoorActor::ADoorActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// Door panel mesh, child of root frame
	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(MeshComponent);

	InteractionText = NSLOCTEXT("Interaction", "DoorOpen", "Open");
	InteractionTag = SereneTags::TAG_Interaction_Door;
	LockedText = NSLOCTEXT("Interaction", "DoorLocked", "Locked");
}

bool ADoorActor::CanInteract_Implementation(AActor* Interactor) const
{
	// Check base enable flag
	if (!bCanBeInteracted)
	{
		return false;
	}

	// Always allow interaction attempt for locked doors (player gets feedback)
	return true;
}

FText ADoorActor::GetInteractionText_Implementation() const
{
	if (bIsLocked)
	{
		return LockedText;
	}

	return Super::GetInteractionText_Implementation();
}

void ADoorActor::OnInteract_Implementation(AActor* Interactor)
{
	// Handle locked door first
	if (bIsLocked)
	{
		// Check if player has the required key
		UInventoryComponent* Inventory = Interactor ? Interactor->FindComponentByClass<UInventoryComponent>() : nullptr;

		if (Inventory && Inventory->HasItem(RequiredItemId))
		{
			// Consume the key
			Inventory->RemoveItemByName(RequiredItemId);

			// Unlock the door
			bIsLocked = false;

			UE_LOG(LogSerene, Log, TEXT("ADoorActor::OnInteract - Door unlocked using %s"),
				*RequiredItemId.ToString());

			// Update interaction text to "Open" (will show on next frame)
			InteractionText = NSLOCTEXT("Interaction", "DoorOpen", "Open");

			// Fall through to open the door
		}
		else
		{
			// No key, door stays locked
			UE_LOG(LogSerene, Log, TEXT("ADoorActor::OnInteract - Door is locked, requires %s"),
				*RequiredItemId.ToString());
			return;
		}
	}

	// Existing open/close toggle logic
	bIsOpen = !bIsOpen;

	if (bIsOpen)
	{
		// Determine which direction the door should swing based on player position.
		if (bOpensTowardPlayer && Interactor)
		{
			const FVector ToPlayer = Interactor->GetActorLocation() - GetActorLocation();
			const float DotProduct = FVector::DotProduct(GetActorForwardVector(), ToPlayer);

			// If player is in front of the door (positive dot), open away (positive angle).
			// If behind, open toward them (negative angle).
			OpenDirection = (DotProduct >= 0.0f) ? 1.0f : -1.0f;
		}

		TargetAngle = OpenAngle * OpenDirection;
		InteractionText = NSLOCTEXT("Interaction", "DoorClose", "Close");

		UE_LOG(LogSerene, Verbose, TEXT("ADoorActor::OnInteract - Door opening. Direction=%.0f, TargetAngle=%.1f"),
			OpenDirection, TargetAngle);
	}
	else
	{
		TargetAngle = 0.0f;
		InteractionText = NSLOCTEXT("Interaction", "DoorOpen", "Open");

		UE_LOG(LogSerene, Verbose, TEXT("ADoorActor::OnInteract - Door closing."));
	}
}

void ADoorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!DoorMesh)
	{
		return;
	}

	// Smoothly interpolate toward target angle
	if (!FMath::IsNearlyEqual(CurrentAngle, TargetAngle, 0.1f))
	{
		CurrentAngle = FMath::FInterpTo(CurrentAngle, TargetAngle, DeltaTime, OpenSpeed);

		// Apply rotation around the Z axis (yaw) as relative rotation on the door mesh
		const FRotator NewRotation(0.0f, CurrentAngle, 0.0f);
		DoorMesh->SetRelativeRotation(NewRotation);
	}
}
