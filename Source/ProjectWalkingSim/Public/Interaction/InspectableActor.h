// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractableBase.h"
#include "InspectableActor.generated.h"

class UDocumentReaderWidget;
class UTexture2D;
class USoundBase;

/**
 * Inspectable story object for environmental storytelling.
 *
 * Photographs, toys, bloodstains, clues -- objects the player can examine
 * to learn about the story. On interaction, opens a full-screen DocumentReaderWidget
 * overlay showing an image and description text.
 *
 * Optionally plays a one-time detective monologue on first inspection.
 *
 * Designers set per-instance:
 *   - InspectionImage: TSoftObjectPtr<UTexture2D> for the full-screen image
 *   - InspectionText: FText description shown below/beside the image
 *   - InspectionMonologue: optional USoundBase for first-inspect voiceover
 *   - DocumentReaderWidgetClass: Blueprint subclass of UDocumentReaderWidget
 */
UCLASS()
class PROJECTWALKINGSIM_API AInspectableActor : public AInteractableBase
{
	GENERATED_BODY()

public:
	AInspectableActor();

protected:
	virtual void OnInteract_Implementation(AActor* Interactor) override;

	/** Full-screen inspection image. TSoftObjectPtr for async-loadable textures. */
	UPROPERTY(EditAnywhere, Category = "Inspectable")
	TSoftObjectPtr<UTexture2D> InspectionImage;

	/** Description text shown alongside the inspection image. */
	UPROPERTY(EditAnywhere, Category = "Inspectable", meta = (MultiLine = true))
	FText InspectionText;

	/** Optional detective monologue played on first inspection only. */
	UPROPERTY(EditAnywhere, Category = "Inspectable")
	TObjectPtr<USoundBase> InspectionMonologue;

	/** Blueprint subclass of UDocumentReaderWidget to create on interaction. */
	UPROPERTY(EditDefaultsOnly, Category = "Inspectable")
	TSubclassOf<UDocumentReaderWidget> DocumentReaderWidgetClass;

private:
	/** Whether this object has been inspected before. Monologue only plays once. */
	bool bHasBeenInspected = false;
};
