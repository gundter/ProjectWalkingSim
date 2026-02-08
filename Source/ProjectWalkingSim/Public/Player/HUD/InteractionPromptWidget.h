// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionPromptWidget.generated.h"

class UTextBlock;

/**
 * HUD widget displaying the interaction prompt (e.g., "E: Open").
 *
 * C++ base class that defines logic for showing/hiding the prompt text.
 * Requires a UMG Blueprint subclass (WBP_InteractionPrompt) with two
 * UTextBlock widgets named exactly:
 *   - "PromptText"   -- displays the interaction action text (e.g., "E: Open")
 *   - "ReticleText"  -- displays the reticle dot (e.g., a centered dot character)
 *
 * These must match the BindWidget meta property names. The UMG Blueprint
 * defines the visual layout (positioning, fonts, colors); this C++ class
 * defines the behavior.
 *
 * Usage:
 *   1. Create WBP_InteractionPrompt in the editor (reparent to this class)
 *   2. Add two TextBlock widgets named "PromptText" and "ReticleText"
 *   3. Bind InteractionComponent::OnInteractableChanged to UpdatePrompt()
 */
UCLASS()
class PROJECTWALKINGSIM_API UInteractionPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Update the prompt with new interactable data.
	 * If Interactable is valid and Text is non-empty, shows prompt.
	 * Otherwise hides it.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void UpdatePrompt(AActor* Interactable, const FText& Text);

	/** Hide the interaction prompt text. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void HidePrompt();

protected:
	/** The text block displaying the interaction prompt (e.g., "E: Open"). Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PromptText;

	/** The reticle dot text block. Must exist in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ReticleText;
};
