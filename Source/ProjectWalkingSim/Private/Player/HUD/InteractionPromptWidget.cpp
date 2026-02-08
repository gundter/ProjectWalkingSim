// Copyright Null Lantern.

#include "Player/HUD/InteractionPromptWidget.h"

#include "Components/TextBlock.h"
#include "Components/Image.h"

void UInteractionPromptWidget::UpdatePrompt(AActor* Interactable, const FText& Text)
{
	if (Interactable && !Text.IsEmpty())
	{
		// Format as "F: {Text}" (e.g., "F: Open", "F: Pick Up", "F: Read")
		const FText FormattedText = FText::Format(
			NSLOCTEXT("InteractionPrompt", "PromptFormat", "F: {0}"), Text);

		if (PromptText)
		{
			PromptText->SetText(FormattedText);
			PromptText->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else
	{
		HidePrompt();
	}
}

void UInteractionPromptWidget::HidePrompt()
{
	if (PromptText)
	{
		PromptText->SetVisibility(ESlateVisibility::Collapsed);
	}
}
