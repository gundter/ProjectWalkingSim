// Copyright Null Lantern.

#include "Interaction/ReadableActor.h"

#include "Tags/SereneTags.h"
#include "Core/SereneLogChannels.h"
#include "Player/HUD/DocumentReaderWidget.h"

AReadableActor::AReadableActor()
{
	InteractionText = NSLOCTEXT("Interaction", "Read", "Read");
	InteractionTag = SereneTags::TAG_Interaction_Readable;
}

void AReadableActor::OnInteract_Implementation(AActor* Interactor)
{
	APawn* InteractorPawn = Cast<APawn>(Interactor);
	if (!InteractorPawn)
	{
		UE_LOG(LogSerene, Warning, TEXT("AReadableActor::OnInteract - Interactor is not a Pawn"));
		return;
	}

	APlayerController* PC = Cast<APlayerController>(InteractorPawn->GetController());
	if (!PC)
	{
		UE_LOG(LogSerene, Warning, TEXT("AReadableActor::OnInteract - No PlayerController found"));
		return;
	}

	if (!DocumentReaderWidgetClass)
	{
		UE_LOG(LogSerene, Warning, TEXT("AReadableActor::OnInteract - DocumentReaderWidgetClass not set on %s"), *GetName());
		return;
	}

	UDocumentReaderWidget* ReaderWidget = CreateWidget<UDocumentReaderWidget>(PC, DocumentReaderWidgetClass);
	if (!ReaderWidget)
	{
		UE_LOG(LogSerene, Error, TEXT("AReadableActor::OnInteract - Failed to create DocumentReaderWidget"));
		return;
	}

	ReaderWidget->AddToViewport(50);
	ReaderWidget->ShowDocument(ReadableTitle, ReadableContent);

	UE_LOG(LogSerene, Log, TEXT("AReadableActor::OnInteract - Reading: %s"), *ReadableTitle.ToString());
}
