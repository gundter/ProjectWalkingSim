// Copyright Null Lantern.

#include "Interaction/DrawerActor.h"

#include "Tags/SereneTags.h"
#include "Core/SereneLogChannels.h"

ADrawerActor::ADrawerActor()
{
	PrimaryActorTick.bCanEverTick = true;

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
}
