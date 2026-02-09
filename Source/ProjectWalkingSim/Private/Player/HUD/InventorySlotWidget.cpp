// Copyright Null Lantern.

#include "Player/HUD/InventorySlotWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Inventory/ItemDataAsset.h"
#include "Inventory/InventoryTypes.h"
#include "Core/SereneLogChannels.h"

void UInventorySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Start in empty state
	ClearSlot();
}

void UInventorySlotWidget::SetSlotData(const FInventorySlot& SlotData, const UItemDataAsset* ItemData)
{
	if (!ItemData)
	{
		UE_LOG(LogSerene, Warning, TEXT("UInventorySlotWidget::SetSlotData - ItemData is null for slot %d"), SlotIndex);
		ClearSlot();
		return;
	}

	// Load and set icon
	if (ItemIcon)
	{
		if (ItemData->Icon.IsValid())
		{
			UTexture2D* IconTex = ItemData->Icon.LoadSynchronous();
			if (IconTex)
			{
				ItemIcon->SetBrushFromTexture(IconTex);
				ItemIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			}
			else
			{
				UE_LOG(LogSerene, Warning, TEXT("UInventorySlotWidget::SetSlotData - Failed to load icon for item %s"), *ItemData->ItemId.ToString());
				ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
		else
		{
			ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// Set quantity text (show only when quantity > 1)
	if (QuantityText)
	{
		if (SlotData.Quantity > 1)
		{
			QuantityText->SetText(FText::AsNumber(SlotData.Quantity));
			QuantityText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			QuantityText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UInventorySlotWidget::ClearSlot()
{
	if (ItemIcon)
	{
		ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (QuantityText)
	{
		QuantityText->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (SelectionHighlight)
	{
		SelectionHighlight->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UInventorySlotWidget::SetSelected(bool bSelected)
{
	if (SelectionHighlight)
	{
		SelectionHighlight->SetVisibility(bSelected ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
	// If SelectionHighlight is nullptr (optional), do nothing
}

FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OnSlotClicked.Broadcast(SlotIndex);
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}
