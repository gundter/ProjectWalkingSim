// Copyright Null Lantern.

#include "Interaction/DrawerActor.h"

#include "Save/SereneSaveGame.h"
#include "Tags/SereneTags.h"
#include "Core/SereneLogChannels.h"

ADrawerActor::ADrawerActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Drawer mesh, child of root frame
	DrawerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DrawerMesh"));
	DrawerMesh->SetupAttachment(MeshComponent);

	InteractionText = NSLOCTEXT("Interaction", "DrawerOpen", "Open");
	InteractionTag = SereneTags::TAG_Interaction_Drawer;
}

void ADrawerActor::OnInteract_Implementation(AActor* Interactor)
{
	bIsOpen = !bIsOpen;

	if (bIsOpen)
	{
		InteractionText = NSLOCTEXT("Interaction", "DrawerClose", "Close");
		UE_LOG(LogSerene, Verbose, TEXT("ADrawerActor::OnInteract - Drawer opening."));
	}
	else
	{
		InteractionText = NSLOCTEXT("Interaction", "DrawerOpen", "Open");
		UE_LOG(LogSerene, Verbose, TEXT("ADrawerActor::OnInteract - Drawer closing."));
	}

	SetActorTickEnabled(true);
}

void ADrawerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!DrawerMesh)
	{
		return;
	}

	const float TargetSlide = bIsOpen ? OpenDistance : 0.0f;

	// Smoothly interpolate toward target slide position
	if (!FMath::IsNearlyEqual(CurrentSlide, TargetSlide, 0.05f))
	{
		CurrentSlide = FMath::FInterpTo(CurrentSlide, TargetSlide, DeltaTime, OpenSpeed);

		// Slide along local X axis (forward out of the frame)
		FVector NewLocation = DrawerInitialLocation;
		NewLocation.X += CurrentSlide;
		DrawerMesh->SetRelativeLocation(NewLocation);
	}
	else
	{
		// Snap to target and stop ticking until next interaction
		CurrentSlide = TargetSlide;
		FVector FinalLocation = DrawerInitialLocation;
		FinalLocation.X += CurrentSlide;
		DrawerMesh->SetRelativeLocation(FinalLocation);
		SetActorTickEnabled(false);
	}
}

// ---------------------------------------------------------------------------
// ISaveable
// ---------------------------------------------------------------------------

FName ADrawerActor::GetSaveId_Implementation() const
{
	return GetFName();
}

void ADrawerActor::WriteSaveData_Implementation(USereneSaveGame* SaveGame)
{
	if (!SaveGame)
	{
		return;
	}

	FSavedDrawerState State;
	State.DrawerId = GetFName();
	State.bIsOpen = bIsOpen;
	State.CurrentSlide = CurrentSlide;

	SaveGame->DrawerStates.Add(State);
}

void ADrawerActor::ReadSaveData_Implementation(USereneSaveGame* SaveGame)
{
	if (!SaveGame)
	{
		return;
	}

	const FName MyId = GetFName();
	for (const FSavedDrawerState& State : SaveGame->DrawerStates)
	{
		if (State.DrawerId == MyId)
		{
			bIsOpen = State.bIsOpen;
			CurrentSlide = State.CurrentSlide;

			// Snap drawer mesh to saved position (no interpolation on load)
			if (DrawerMesh)
			{
				FVector NewLocation = DrawerInitialLocation;
				NewLocation.X += CurrentSlide;
				DrawerMesh->SetRelativeLocation(NewLocation);
			}

			// Update interaction text to match state
			InteractionText = bIsOpen
				? NSLOCTEXT("Interaction", "DrawerClose", "Close")
				: NSLOCTEXT("Interaction", "DrawerOpen", "Open");

			UE_LOG(LogSerene, Verbose, TEXT("ADrawerActor [%s]: Restored from save (open=%d, slide=%.1f)"),
				*MyId.ToString(), bIsOpen, CurrentSlide);
			return;
		}
	}
}
