// Copyright Null Lantern.

#include "Player/HUD/ItemTooltipWidget.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Inventory/ItemDataAsset.h"
#include "Core/SereneLogChannels.h"

void UItemTooltipWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Start hidden (use Visibility to prevent input when hidden)
	SetVisibility(ESlateVisibility::Collapsed);

	// Bind button click events
	if (UseButton)
	{
		UseButton->OnClicked.AddDynamic(this, &UItemTooltipWidget::HandleUseClicked);
	}

	if (CombineButton)
	{
		CombineButton->OnClicked.AddDynamic(this, &UItemTooltipWidget::HandleCombineClicked);
	}

	if (InfoButton)
	{
		InfoButton->OnClicked.AddDynamic(this, &UItemTooltipWidget::HandleInfoClicked);
	}

	if (DiscardButton)
	{
		DiscardButton->OnClicked.AddDynamic(this, &UItemTooltipWidget::HandleDiscardClicked);
	}
}

void UItemTooltipWidget::SetTooltipData(int32 InSlotIndex, const UItemDataAsset* ItemData)
{
	BoundSlotIndex = InSlotIndex;
	CachedItemData = ItemData;

	// Clear any pending confirm mode when new data is set
	bInDiscardConfirmMode = false;

	if (!ItemData)
	{
		UE_LOG(LogSerene, Warning, TEXT("UItemTooltipWidget::SetTooltipData - ItemData is null"));
		return;
	}

	if (ItemNameText)
	{
		ItemNameText->SetText(ItemData->DisplayName);
	}

	if (ItemDescriptionText)
	{
		ItemDescriptionText->SetText(ItemData->Description);
	}
}

void UItemTooltipWidget::SetDiscardConfirmMode(bool bConfirmMode, const UItemDataAsset* ItemData)
{
	bInDiscardConfirmMode = bConfirmMode;

	if (ItemDescriptionText)
	{
		if (bConfirmMode)
		{
			// Show warning message - this is a key item
			ItemDescriptionText->SetText(NSLOCTEXT("Inventory", "DiscardKeyItemWarning",
				"WARNING: This is a KEY ITEM needed for progression.\n\nClick Discard again to confirm."));
		}
		else if (ItemData)
		{
			// Restore normal description
			ItemDescriptionText->SetText(ItemData->Description);
		}
		else if (CachedItemData)
		{
			// Fallback to cached data
			ItemDescriptionText->SetText(CachedItemData->Description);
		}
	}
}

void UItemTooltipWidget::HideTooltip()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UItemTooltipWidget::ShowTooltip()
{
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UItemTooltipWidget::HandleUseClicked()
{
	OnUseClicked.Broadcast(BoundSlotIndex);
}

void UItemTooltipWidget::HandleCombineClicked()
{
	OnCombineClicked.Broadcast(BoundSlotIndex);
}

void UItemTooltipWidget::HandleInfoClicked()
{
	OnInfoClicked.Broadcast(BoundSlotIndex);
}

void UItemTooltipWidget::HandleDiscardClicked()
{
	OnDiscardClicked.Broadcast(BoundSlotIndex);
}
