// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractableBase.h"
#include "ReadableActor.generated.h"

/**
 * Readable actor for notes, documents, and letters in The Juniper Tree.
 *
 * Contains a title and multi-line content text. Currently logs the title
 * on interaction. Full text display UI (popup widget) will be added in a
 * later phase when the document reading system is built.
 *
 * Designers populate ReadableTitle and ReadableContent per instance.
 */
UCLASS()
class PROJECTWALKINGSIM_API AReadableActor : public AInteractableBase
{
	GENERATED_BODY()

public:
	AReadableActor();

protected:
	virtual void OnInteract_Implementation(AActor* Interactor) override;

	/** Title of the document. Shown in log and future UI header. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Readable")
	FText ReadableTitle;

	/** Full document content. Multi-line text for future display UI. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Readable", meta = (MultiLine = true))
	FText ReadableContent;
};
