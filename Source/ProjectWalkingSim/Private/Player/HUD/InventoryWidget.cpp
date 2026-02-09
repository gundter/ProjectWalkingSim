// Copyright Null Lantern.

#include "Player/HUD/InventoryWidget.h"

#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Player/HUD/InventorySlotWidget.h"
#include "Player/HUD/ItemTooltipWidget.h"
#include "Inventory/InventoryTypes.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemDataAsset.h"
#include "Core/SereneLogChannels.h"

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Start hidden per UMG conventions
	SetRenderOpacity(0.0f);
	bIsVisible = false;

	// Create 8 slot widgets
	if (!SlotWidgetClass)
	{
		UE_LOG(LogSerene, Error, TEXT("UInventoryWidget::NativeConstruct - SlotWidgetClass is not set! Set it to WBP_InventorySlot in Blueprint defaults."));
		return;
	}

	if (!SlotContainer)
	{
		UE_LOG(LogSerene, Error, TEXT("UInventoryWidget::NativeConstruct - SlotContainer is null! Check BindWidget name in WBP."));
		return;
	}

	SlotWidgets.Empty();
	SlotWidgets.Reserve(UInventoryComponent::MaxSlots);

	for (int32 i = 0; i < UInventoryComponent::MaxSlots; ++i)
	{
		UInventorySlotWidget* SlotWidget = CreateWidget<UInventorySlotWidget>(this, SlotWidgetClass);
		if (SlotWidget)
		{
			SlotWidget->SlotIndex = i;
			SlotWidget->OnSlotClicked.AddDynamic(this, &UInventoryWidget::HandleSlotClicked);
			SlotContainer->AddChildToHorizontalBox(SlotWidget);
			SlotWidgets.Add(SlotWidget);
		}
		else
		{
			UE_LOG(LogSerene, Error, TEXT("UInventoryWidget::NativeConstruct - Failed to create slot widget %d"), i);
		}
	}

	// Bind tooltip action delegates for re-broadcasting
	if (ItemTooltip)
	{
		ItemTooltip->OnUseClicked.AddDynamic(this, &UInventoryWidget::HandleUseClicked);
		ItemTooltip->OnCombineClicked.AddDynamic(this, &UInventoryWidget::HandleCombineClicked);
		ItemTooltip->OnDiscardClicked.AddDynamic(this, &UInventoryWidget::HandleDiscardClicked);
	}
	else
	{
		UE_LOG(LogSerene, Warning, TEXT("UInventoryWidget::NativeConstruct - ItemTooltip is null! Check BindWidget name in WBP."));
	}

	UE_LOG(LogSerene, Log, TEXT("UInventoryWidget::NativeConstruct - Created %d slot widgets"), SlotWidgets.Num());
}

void UInventoryWidget::RefreshSlots(const TArray<FInventorySlot>& Slots, const UInventoryComponent* InventoryComp)
{
	CachedInventoryComp = InventoryComp;

	const int32 NumSlots = FMath::Min(Slots.Num(), SlotWidgets.Num());

	for (int32 i = 0; i < NumSlots; ++i)
	{
		const FInventorySlot& SlotData = Slots[i];
		UInventorySlotWidget* SlotWidget = SlotWidgets[i];

		if (!SlotWidget)
		{
			continue;
		}

		if (SlotData.IsEmpty())
		{
			SlotWidget->ClearSlot();

			// If selected slot becomes empty, deselect
			if (SelectedSlotIndex == i)
			{
				DeselectSlot();
			}
		}
		else
		{
			const UItemDataAsset* ItemData = InventoryComp ? InventoryComp->GetItemData(SlotData.ItemId) : nullptr;
			SlotWidget->SetSlotData(SlotData, ItemData);
		}
	}
}

void UInventoryWidget::ShowInventory()
{
	SetRenderOpacity(1.0f);
	bIsVisible = true;
	UE_LOG(LogSerene, Log, TEXT("UInventoryWidget::ShowInventory - Inventory panel now visible"));
}

void UInventoryWidget::HideInventory()
{
	SetRenderOpacity(0.0f);
	bIsVisible = false;

	// Deselect slot and hide tooltip when hiding inventory
	DeselectSlot();

	UE_LOG(LogSerene, Log, TEXT("UInventoryWidget::HideInventory - Inventory panel hidden"));
}

void UInventoryWidget::SelectSlot(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= SlotWidgets.Num())
	{
		UE_LOG(LogSerene, Warning, TEXT("UInventoryWidget::SelectSlot - Invalid slot index: %d"), SlotIndex);
		return;
	}

	// Deselect previous slot if different
	if (SelectedSlotIndex >= 0 && SelectedSlotIndex < SlotWidgets.Num() && SelectedSlotIndex != SlotIndex)
	{
		if (SlotWidgets[SelectedSlotIndex])
		{
			SlotWidgets[SelectedSlotIndex]->SetSelected(false);
		}
	}

	SelectedSlotIndex = SlotIndex;

	// Highlight the new slot
	if (SlotWidgets[SlotIndex])
	{
		SlotWidgets[SlotIndex]->SetSelected(true);
	}

	// Show tooltip with item data
	if (ItemTooltip && CachedInventoryComp)
	{
		const TArray<FInventorySlot>& Slots = CachedInventoryComp->GetSlots();
		if (SlotIndex < Slots.Num() && !Slots[SlotIndex].IsEmpty())
		{
			const UItemDataAsset* ItemData = CachedInventoryComp->GetItemData(Slots[SlotIndex].ItemId);
			ItemTooltip->SetTooltipData(SlotIndex, ItemData);
			ItemTooltip->ShowTooltip();
		}
		else
		{
			// Slot is empty, hide tooltip
			ItemTooltip->HideTooltip();
		}
	}
}

void UInventoryWidget::DeselectSlot()
{
	// Remove highlight from current selection
	if (SelectedSlotIndex >= 0 && SelectedSlotIndex < SlotWidgets.Num())
	{
		if (SlotWidgets[SelectedSlotIndex])
		{
			SlotWidgets[SelectedSlotIndex]->SetSelected(false);
		}
	}

	SelectedSlotIndex = -1;

	// Hide tooltip
	if (ItemTooltip)
	{
		ItemTooltip->HideTooltip();
	}
}

void UInventoryWidget::HandleSlotClicked(int32 SlotIndex)
{
	// Toggle behavior: clicking same slot deselects, clicking different slot selects
	if (SelectedSlotIndex == SlotIndex)
	{
		DeselectSlot();
	}
	else
	{
		SelectSlot(SlotIndex);
	}
}

void UInventoryWidget::HandleUseClicked(int32 SlotIndex)
{
	OnUseRequested.Broadcast(SlotIndex);
}

void UInventoryWidget::HandleCombineClicked(int32 SlotIndex)
{
	OnCombineRequested.Broadcast(SlotIndex);
}

void UInventoryWidget::HandleDiscardClicked(int32 SlotIndex)
{
	OnDiscardRequested.Broadcast(SlotIndex);
}
