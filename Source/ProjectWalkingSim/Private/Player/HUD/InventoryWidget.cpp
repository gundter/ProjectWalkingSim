// Copyright Null Lantern.

#include "Player/HUD/InventoryWidget.h"

#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Player/HUD/InventorySlotWidget.h"
#include "Player/HUD/ItemTooltipWidget.h"
#include "Player/SerenePlayerController.h"
#include "Inventory/InventoryTypes.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemDataAsset.h"
#include "Core/SereneLogChannels.h"
#include "InputCoreTypes.h"

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Start hidden per UMG conventions
	SetRenderOpacity(0.0f);
	bIsVisible = false;

	// Enable keyboard focus for NativeOnKeyDown
	SetIsFocusable(true);

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

void UInventoryWidget::RefreshSlots(const TArray<FInventorySlot>& Slots, UInventoryComponent* InventoryComp)
{
	CachedInventoryComp = InventoryComp;

	UE_LOG(LogSerene, Log, TEXT("UInventoryWidget::RefreshSlots - Slots.Num()=%d, SlotWidgets.Num()=%d, InventoryComp=%s"),
		Slots.Num(), SlotWidgets.Num(), InventoryComp ? TEXT("valid") : TEXT("null"));

	const int32 NumSlots = FMath::Min(Slots.Num(), SlotWidgets.Num());

	for (int32 i = 0; i < NumSlots; ++i)
	{
		const FInventorySlot& SlotData = Slots[i];
		UInventorySlotWidget* SlotWidget = SlotWidgets[i];

		if (!SlotWidget)
		{
			UE_LOG(LogSerene, Warning, TEXT("  Slot %d: SlotWidget is null"), i);
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
			UE_LOG(LogSerene, Log, TEXT("  Slot %d: ItemId=%s, Quantity=%d"), i, *SlotData.ItemId.ToString(), SlotData.Quantity);
			const UItemDataAsset* ItemData = InventoryComp ? InventoryComp->GetItemData(SlotData.ItemId) : nullptr;
			SlotWidget->SetSlotData(SlotData, ItemData);
		}
	}
}

void UInventoryWidget::ShowInventory()
{
	SetRenderOpacity(1.0f);
	bIsVisible = true;

	// Set keyboard focus so NativeOnKeyDown fires
	SetKeyboardFocus();

	UE_LOG(LogSerene, Log, TEXT("UInventoryWidget::ShowInventory - Inventory panel now visible"));
}

void UInventoryWidget::HideInventory()
{
	SetRenderOpacity(0.0f);
	bIsVisible = false;

	// Exit combine mode if active
	if (bInCombineMode)
	{
		ExitCombineMode();
	}

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
	// If in combine mode, handle second slot selection
	if (bInCombineMode)
	{
		// Same slot clicked - cancel combine mode
		if (SlotIndex == CombineSourceSlotIndex)
		{
			UE_LOG(LogSerene, Verbose, TEXT("UInventoryWidget::HandleSlotClicked - Same slot clicked, exiting combine mode"));
			ExitCombineMode();
			return;
		}

		// Check if clicked slot is empty
		if (CachedInventoryComp)
		{
			const TArray<FInventorySlot>& Slots = CachedInventoryComp->GetSlots();
			if (SlotIndex < Slots.Num() && Slots[SlotIndex].IsEmpty())
			{
				// Empty slot clicked during combine mode - exit combine mode
				UE_LOG(LogSerene, Verbose, TEXT("UInventoryWidget::HandleSlotClicked - Empty slot clicked, exiting combine mode"));
				ExitCombineMode();
				return;
			}
		}

		// Valid second slot - broadcast combine request
		UE_LOG(LogSerene, Log, TEXT("UInventoryWidget::HandleSlotClicked - Combine slots %d + %d"), CombineSourceSlotIndex, SlotIndex);
		OnCombineSlotSelected.Broadcast(CombineSourceSlotIndex, SlotIndex);
		ExitCombineMode();
		return;
	}

	// Normal mode: Toggle behavior - clicking same slot deselects, clicking different slot selects
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

void UInventoryWidget::EnterCombineMode(int32 SourceSlotIndex)
{
	bInCombineMode = true;
	CombineSourceSlotIndex = SourceSlotIndex;

	// Ensure source slot is selected and highlighted
	SelectSlot(SourceSlotIndex);

	UE_LOG(LogSerene, Log, TEXT("UInventoryWidget::EnterCombineMode - Entered combine mode with source slot %d"), SourceSlotIndex);
}

void UInventoryWidget::ExitCombineMode()
{
	bInCombineMode = false;
	CombineSourceSlotIndex = -1;

	UE_LOG(LogSerene, Verbose, TEXT("UInventoryWidget::ExitCombineMode - Exited combine mode"));
}

void UInventoryWidget::SetTooltipDiscardConfirmMode(bool bConfirmMode, const UItemDataAsset* ItemData)
{
	if (ItemTooltip)
	{
		ItemTooltip->SetDiscardConfirmMode(bConfirmMode, ItemData);
	}
}

FReply UInventoryWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	// Number keys 1-8 for direct slot selection
	static const FKey NumberKeys[] = {
		EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four,
		EKeys::Five, EKeys::Six, EKeys::Seven, EKeys::Eight
	};

	for (int32 i = 0; i < 8; i++)
	{
		if (Key == NumberKeys[i])
		{
			UE_LOG(LogSerene, Verbose, TEXT("UInventoryWidget::NativeOnKeyDown - Number key %d pressed, selecting slot %d"), i + 1, i);
			SelectSlot(i);
			return FReply::Handled();
		}
	}

	// Arrow keys for cycling
	if (Key == EKeys::Left)
	{
		UE_LOG(LogSerene, Verbose, TEXT("UInventoryWidget::NativeOnKeyDown - Left arrow pressed"));
		NavigateSlot(-1);
		return FReply::Handled();
	}

	if (Key == EKeys::Right)
	{
		UE_LOG(LogSerene, Verbose, TEXT("UInventoryWidget::NativeOnKeyDown - Right arrow pressed"));
		NavigateSlot(1);
		return FReply::Handled();
	}

	// Escape to close combine mode or deselect
	if (Key == EKeys::Escape)
	{
		if (bInCombineMode)
		{
			UE_LOG(LogSerene, Verbose, TEXT("UInventoryWidget::NativeOnKeyDown - Escape pressed, exiting combine mode"));
			ExitCombineMode();
		}
		else
		{
			UE_LOG(LogSerene, Verbose, TEXT("UInventoryWidget::NativeOnKeyDown - Escape pressed, deselecting slot"));
			DeselectSlot();
		}
		return FReply::Handled();
	}

	// Tab key to close inventory (pass through to controller)
	if (Key == EKeys::Tab)
	{
		if (ASerenePlayerController* PC = Cast<ASerenePlayerController>(GetOwningPlayer()))
		{
			UE_LOG(LogSerene, Verbose, TEXT("UInventoryWidget::NativeOnKeyDown - Tab pressed, closing inventory"));
			PC->CloseInventory();
			return FReply::Handled();
		}
	}

	// Delete key for discard
	if (Key == EKeys::Delete && SelectedSlotIndex >= 0)
	{
		UE_LOG(LogSerene, Verbose, TEXT("UInventoryWidget::NativeOnKeyDown - Delete pressed, discarding slot %d"), SelectedSlotIndex);
		OnDiscardRequested.Broadcast(SelectedSlotIndex);
		return FReply::Handled();
	}

	// Let unhandled keys pass through (WASD and other game input keys)
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UInventoryWidget::NavigateSlot(int32 Direction)
{
	if (SlotWidgets.Num() == 0)
	{
		return;
	}

	int32 NewIndex;
	if (SelectedSlotIndex < 0)
	{
		// No slot selected - start from first or last depending on direction
		NewIndex = (Direction > 0) ? 0 : SlotWidgets.Num() - 1;
	}
	else
	{
		// Move in direction with wraparound
		NewIndex = SelectedSlotIndex + Direction;
		if (NewIndex < 0)
		{
			NewIndex = SlotWidgets.Num() - 1;
		}
		if (NewIndex >= SlotWidgets.Num())
		{
			NewIndex = 0;
		}
	}

	SelectSlot(NewIndex);
}
