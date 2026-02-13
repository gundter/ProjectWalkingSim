// Copyright Null Lantern.

#include "Interaction/DoorActor.h"

#include "Inventory/InventoryComponent.h"
#include "Save/SereneSaveGame.h"
#include "Tags/SereneTags.h"
#include "Core/SereneLogChannels.h"

ADoorActor::ADoorActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

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

	SetActorTickEnabled(true);
}

void ADoorActor::OpenForAI(AActor* AIActor)
{
	// AI cannot open locked doors
	if (bIsLocked)
	{
		return;
	}

	// Already open -- nothing to do
	if (bIsOpen)
	{
		return;
	}

	bIsOpen = true;

	// Determine swing direction using the same dot product pattern as player interaction
	if (AIActor)
	{
		const FVector ToAI = AIActor->GetActorLocation() - GetActorLocation();
		const float DotProduct = FVector::DotProduct(GetActorForwardVector(), ToAI);
		OpenDirection = (DotProduct >= 0.0f) ? 1.0f : -1.0f;
	}

	TargetAngle = OpenAngle * OpenDirection;
	SetActorTickEnabled(true);

	UE_LOG(LogSerene, Log, TEXT("ADoorActor [%s]: Opened for AI. Direction=%.0f, TargetAngle=%.1f"),
		*GetName(), OpenDirection, TargetAngle);
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
	else
	{
		// Snap to target and stop ticking until next interaction
		CurrentAngle = TargetAngle;
		DoorMesh->SetRelativeRotation(FRotator(0.0f, CurrentAngle, 0.0f));
		SetActorTickEnabled(false);
	}
}

// ---------------------------------------------------------------------------
// ISaveable
// ---------------------------------------------------------------------------

FName ADoorActor::GetSaveId_Implementation() const
{
	return GetFName();
}

void ADoorActor::WriteSaveData_Implementation(USereneSaveGame* SaveGame)
{
	if (!SaveGame)
	{
		return;
	}

	FSavedDoorState State;
	State.DoorId = GetFName();
	State.bIsOpen = bIsOpen;
	State.bIsLocked = bIsLocked;
	State.CurrentAngle = CurrentAngle;
	State.OpenDirection = OpenDirection;

	SaveGame->DoorStates.Add(State);
}

void ADoorActor::ReadSaveData_Implementation(USereneSaveGame* SaveGame)
{
	if (!SaveGame)
	{
		return;
	}

	const FName MyId = GetFName();
	for (const FSavedDoorState& State : SaveGame->DoorStates)
	{
		if (State.DoorId == MyId)
		{
			bIsOpen = State.bIsOpen;
			bIsLocked = State.bIsLocked;
			CurrentAngle = State.CurrentAngle;
			OpenDirection = State.OpenDirection;
			TargetAngle = bIsOpen ? (OpenAngle * OpenDirection) : 0.0f;

			// Snap door mesh to saved rotation (no interpolation on load)
			if (DoorMesh)
			{
				DoorMesh->SetRelativeRotation(FRotator(0.0f, CurrentAngle, 0.0f));
			}

			// Update interaction text to match state
			InteractionText = bIsOpen
				? NSLOCTEXT("Interaction", "DoorClose", "Close")
				: NSLOCTEXT("Interaction", "DoorOpen", "Open");

			UE_LOG(LogSerene, Verbose, TEXT("ADoorActor [%s]: Restored from save (open=%d, locked=%d, angle=%.1f)"),
				*MyId.ToString(), bIsOpen, bIsLocked, CurrentAngle);
			return;
		}
	}
}
