// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractableBase.h"
#include "ReadableActor.generated.h"

class UDocumentReaderWidget;

/**
 * Readable actor for notes, documents, and letters in The Juniper Tree.
 *
 * Contains a title and multi-line content text. On interaction, creates a
 * full-screen DocumentReaderWidget displaying the title and content.
 *
 * Designers populate ReadableTitle and ReadableContent per instance.
 * Assign DocumentReaderWidgetClass in the Blueprint subclass.
 */
UCLASS()
class PROJECTWALKINGSIM_API AReadableActor : public AInteractableBase
{
	GENERATED_BODY()

public:
	AReadableActor();

protected:
	virtual void OnInteract_Implementation(AActor* Interactor) override;

	/** Title of the document. Shown in the document reader header. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Readable")
	FText ReadableTitle;

	/** Full document content. Multi-line text displayed in the document reader. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Readable", meta = (MultiLine = true))
	FText ReadableContent;

	/** Blueprint subclass of UDocumentReaderWidget to create on interaction. */
	UPROPERTY(EditDefaultsOnly, Category = "Readable")
	TSubclassOf<UDocumentReaderWidget> DocumentReaderWidgetClass;
};
