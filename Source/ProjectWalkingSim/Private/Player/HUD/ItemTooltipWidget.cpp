// Copyright Null Lantern.

#include "Player/HUD/ItemTooltipWidget.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Inventory/ItemDataAsset.h"
#include "Core/SereneLogChannels.h"

void UItemTooltipWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Start hidden
	SetRenderOpacity(0.0f);

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

void UItemTooltipWidget::HideTooltip()
{
	SetRenderOpacity(0.0f);
}

void UItemTooltipWidget::ShowTooltip()
{
	SetRenderOpacity(1.0f);
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
