// Copyright Null Lantern.

#include "Player/HUD/DocumentReaderWidget.h"

#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Core/SereneLogChannels.h"

void UDocumentReaderWidget::ShowDocument(const FText& Title, const FText& Content)
{
	if (TitleText)
	{
		TitleText->SetText(Title);
		TitleText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (ContentText)
	{
		ContentText->SetText(Content);
	}

	if (InspectionImage)
	{
		InspectionImage->SetVisibility(ESlateVisibility::Collapsed);
	}

	bIsShowingDocument = true;

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		PC->SetInputMode(InputMode);
	}

	SetKeyboardFocus();

	UE_LOG(LogSerene, Log, TEXT("UDocumentReaderWidget::ShowDocument - Showing: %s"), *Title.ToString());
}

void UDocumentReaderWidget::ShowInspection(const FText& Description, UTexture2D* Image)
{
	if (TitleText)
	{
		TitleText->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (ContentText)
	{
		ContentText->SetText(Description);
	}

	if (InspectionImage)
	{
		if (Image)
		{
			InspectionImage->SetBrushFromTexture(Image);
			InspectionImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			InspectionImage->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	bIsShowingDocument = true;

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		PC->SetInputMode(InputMode);
	}

	SetKeyboardFocus();

	UE_LOG(LogSerene, Log, TEXT("UDocumentReaderWidget::ShowInspection - Showing inspection overlay"));
}

void UDocumentReaderWidget::CloseDocument()
{
	if (!bIsShowingDocument)
	{
		return;
	}

	bIsShowingDocument = false;

	RemoveFromParent();

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
	}

	OnDocumentClosed.Broadcast();

	UE_LOG(LogSerene, Log, TEXT("UDocumentReaderWidget::CloseDocument - Document closed"));
}

FReply UDocumentReaderWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey PressedKey = InKeyEvent.GetKey();

	if (PressedKey == EKeys::E || PressedKey == EKeys::Escape)
	{
		CloseDocument();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
