// Copyright Null Lantern.

#include "Player/HUD/InteractionPromptWidget.h"

#include "Components/TextBlock.h"

void UInteractionPromptWidget::UpdatePrompt(AActor* Interactable, const FText& Text)
{
	if (Interactable && !Text.IsEmpty())
	{
		// Format as "E: {Text}" (e.g., "E: Open", "E: Pick Up", "E: Read")
		const FText FormattedText = FText::Format(
			NSLOCTEXT("InteractionPrompt", "PromptFormat", "E: {0}"), Text);

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
