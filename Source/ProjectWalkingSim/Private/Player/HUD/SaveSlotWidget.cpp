// Copyright Null Lantern.

#include "Player/HUD/SaveSlotWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "ImageUtils.h"
#include "Save/SaveTypes.h"
#include "Core/SereneLogChannels.h"

void USaveSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SlotButton)
	{
		SlotButton->OnClicked.AddDynamic(this, &USaveSlotWidget::HandleSlotButtonClicked);
	}
}

void USaveSlotWidget::SetSlotData(int32 InSlotIndex, const FSaveSlotInfo& Info, bool bIsOccupied)
{
	SlotIndex = InSlotIndex;

	// Set slot label (1-indexed for display)
	if (SlotLabelText)
	{
		SlotLabelText->SetText(FText::FromString(FString::Printf(TEXT("Slot %d"), InSlotIndex + 1)));
	}

	if (bIsOccupied)
	{
		// Format timestamp for display
		if (TimestampText)
		{
			const FString FormattedTime = Info.Timestamp.ToString(TEXT("%b %d, %Y %I:%M %p"));
			TimestampText->SetText(FText::FromString(FormattedTime));
		}

		// Reconstruct thumbnail from saved JPEG bytes
		if (ThumbnailImage && Info.ScreenshotData.Num() > 0)
		{
			UTexture2D* Texture = FImageUtils::ImportBufferAsTexture2D(
				TArrayView64<const uint8>(Info.ScreenshotData.GetData(), Info.ScreenshotData.Num()));

			if (Texture)
			{
				ThumbnailImage->SetBrushFromTexture(Texture);
			}
			else
			{
				UE_LOG(LogSerene, Warning, TEXT("USaveSlotWidget::SetSlotData - Failed to reconstruct thumbnail for slot %d"), InSlotIndex);
			}
		}
	}
	else
	{
		// Empty slot
		if (TimestampText)
		{
			TimestampText->SetText(FText::FromString(TEXT("Empty")));
		}

		// Reset thumbnail to default (clear the brush)
		if (ThumbnailImage)
		{
			ThumbnailImage->SetBrushFromTexture(nullptr);
		}
	}
}

void USaveSlotWidget::HandleSlotButtonClicked()
{
	OnSlotClicked.Broadcast(SlotIndex);
}
