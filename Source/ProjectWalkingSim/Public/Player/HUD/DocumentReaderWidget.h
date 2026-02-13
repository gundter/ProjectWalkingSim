// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DocumentReaderWidget.generated.h"

class UTextBlock;
class UImage;
class UTexture2D;

/** Delegate broadcast when the document reader is closed. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDocumentClosed);

/**
 * Full-screen document/inspection reader widget.
 *
 * Supports two display modes:
 *   - Document mode: Title + body text (for ReadableActor notes/letters)
 *   - Inspection mode: Image + description text (for InspectableActor objects)
 *
 * Created per-use by the actor that triggers it, added to viewport, and removed on close.
 * Player presses E or Esc to close. Input mode switches to UI-only while open.
 *
 * C++ base class for a UMG Blueprint subclass (WBP_DocumentReader) with:
 *   - "TitleText"       (UTextBlock) -- document title (hidden in inspection mode)
 *   - "ContentText"     (UTextBlock) -- document body or inspection description
 *   - "InspectionImage" (UImage)     -- full-screen image (hidden in document mode)
 *
 * Usage:
 *   1. Create WBP_DocumentReader in editor (reparent to UDocumentReaderWidget)
 *   2. Add the required widgets with matching names
 *   3. ReadableActor/InspectableActor creates widget instance on interaction
 */
UCLASS()
class PROJECTWALKINGSIM_API UDocumentReaderWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Broadcast when the document reader is closed. */
	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOnDocumentClosed OnDocumentClosed;

	/** Show a readable document with title and body text. */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowDocument(const FText& Title, const FText& Content);

	/** Show an inspection overlay with image and description text. */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowInspection(const FText& Description, UTexture2D* Image);

	/** Close the document reader, restore game input, and remove from viewport. */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void CloseDocument();

protected:
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	/** Document title text. Visible in document mode, collapsed in inspection mode. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TitleText;

	/** Document body text or inspection description. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ContentText;

	/** Full-screen inspection image. Collapsed in document mode. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> InspectionImage;

private:
	/** Whether the widget is currently displaying content. Guards against double-close. */
	bool bIsShowingDocument = false;
};
