// Copyright Null Lantern.

#include "Interaction/InspectableActor.h"

#include "Tags/SereneTags.h"
#include "Core/SereneLogChannels.h"
#include "Player/HUD/DocumentReaderWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Texture2D.h"

AInspectableActor::AInspectableActor()
{
	InteractionText = NSLOCTEXT("Interaction", "Inspect", "Inspect");
	InteractionTag = SereneTags::TAG_Interaction_Inspectable;
}

void AInspectableActor::OnInteract_Implementation(AActor* Interactor)
{
	APawn* InteractorPawn = Cast<APawn>(Interactor);
	if (!InteractorPawn)
	{
		UE_LOG(LogSerene, Warning, TEXT("AInspectableActor::OnInteract - Interactor is not a Pawn"));
		return;
	}

	APlayerController* PC = Cast<APlayerController>(InteractorPawn->GetController());
	if (!PC)
	{
		UE_LOG(LogSerene, Warning, TEXT("AInspectableActor::OnInteract - No PlayerController found"));
		return;
	}

	if (!DocumentReaderWidgetClass)
	{
		UE_LOG(LogSerene, Warning, TEXT("AInspectableActor::OnInteract - DocumentReaderWidgetClass not set on %s"), *GetName());
		return;
	}

	// Load the inspection image if a path is set
	UTexture2D* LoadedTexture = nullptr;
	if (!InspectionImage.IsNull())
	{
		LoadedTexture = InspectionImage.LoadSynchronous();
	}

	// Create the document reader widget and add to viewport
	UDocumentReaderWidget* Widget = CreateWidget<UDocumentReaderWidget>(PC, DocumentReaderWidgetClass);
	if (!Widget)
	{
		UE_LOG(LogSerene, Warning, TEXT("AInspectableActor::OnInteract - Failed to create DocumentReaderWidget"));
		return;
	}

	Widget->AddToViewport(50);
	Widget->ShowInspection(InspectionText, LoadedTexture);

	// Play optional one-time monologue
	if (InspectionMonologue && !bHasBeenInspected)
	{
		UGameplayStatics::PlaySound2D(this, InspectionMonologue);
	}

	bHasBeenInspected = true;

	UE_LOG(LogSerene, Log, TEXT("AInspectableActor::OnInteract - Inspecting: %s"), *GetName());
}
