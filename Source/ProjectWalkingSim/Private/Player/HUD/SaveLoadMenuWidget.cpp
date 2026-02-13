// Copyright Null Lantern.

#include "Player/HUD/SaveLoadMenuWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Player/HUD/SaveSlotWidget.h"
#include "Save/SaveSubsystem.h"
#include "Save/SaveTypes.h"
#include "Core/SereneLogChannels.h"

void USaveLoadMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind slot click delegates
	if (Slot0)
	{
		Slot0->OnSlotClicked.AddDynamic(this, &USaveLoadMenuWidget::HandleSlotClicked);
	}
	if (Slot1)
	{
		Slot1->OnSlotClicked.AddDynamic(this, &USaveLoadMenuWidget::HandleSlotClicked);
	}
	if (Slot2)
	{
		Slot2->OnSlotClicked.AddDynamic(this, &USaveLoadMenuWidget::HandleSlotClicked);
	}

	// Bind confirmation buttons
	if (ConfirmYesButton)
	{
		ConfirmYesButton->OnClicked.AddDynamic(this, &USaveLoadMenuWidget::HandleConfirmYes);
	}
	if (ConfirmNoButton)
	{
		ConfirmNoButton->OnClicked.AddDynamic(this, &USaveLoadMenuWidget::HandleConfirmNo);
	}

	// Bind back button
	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &USaveLoadMenuWidget::HandleBackClicked);
	}

	// Start with confirmation overlay hidden
	if (ConfirmOverlay)
	{
		ConfirmOverlay->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void USaveLoadMenuWidget::OpenMenu(ESaveLoadMode Mode)
{
	CurrentMode = Mode;
	PendingOverwriteSlotIndex = -1;

	// Update title
	if (MenuTitleText)
	{
		const FText Title = (Mode == ESaveLoadMode::Save)
			? FText::FromString(TEXT("Save Game"))
			: FText::FromString(TEXT("Load Game"));
		MenuTitleText->SetText(Title);
	}

	// Update back button text
	if (BackButtonText)
	{
		BackButtonText->SetText(FText::FromString(TEXT("Back")));
	}

	// Hide confirmation overlay
	if (ConfirmOverlay)
	{
		ConfirmOverlay->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Refresh slot data
	RefreshSlots();

	// Show the widget
	SetVisibility(ESlateVisibility::Visible);
}

void USaveLoadMenuWidget::CloseMenu()
{
	SetVisibility(ESlateVisibility::Collapsed);
	PendingOverwriteSlotIndex = -1;
	OnMenuClosed.Broadcast();
}

void USaveLoadMenuWidget::HandleSlotClicked(int32 SlotIndex)
{
	USaveSubsystem* SaveSub = GetGameInstance()->GetSubsystem<USaveSubsystem>();
	if (!SaveSub)
	{
		UE_LOG(LogSerene, Error, TEXT("USaveLoadMenuWidget::HandleSlotClicked - SaveSubsystem is null"));
		return;
	}

	const bool bSlotOccupied = SaveSub->DoesSaveExist(SlotIndex);

	if (CurrentMode == ESaveLoadMode::Save)
	{
		if (bSlotOccupied)
		{
			// Occupied slot in Save mode -- show overwrite confirmation
			PendingOverwriteSlotIndex = SlotIndex;
			if (ConfirmOverlay)
			{
				ConfirmOverlay->SetVisibility(ESlateVisibility::Visible);
			}
		}
		else
		{
			// Empty slot -- save directly
			SaveSub->SaveToSlot(SlotIndex);
			CloseMenu();
		}
	}
	else // Load mode
	{
		if (bSlotOccupied)
		{
			SaveSub->LoadFromSlot(SlotIndex);
			// LoadFromSlot restarts the level; widget will be destroyed
		}
		// Empty slot in Load mode -- ignore click
	}
}

void USaveLoadMenuWidget::HandleConfirmYes()
{
	if (PendingOverwriteSlotIndex < 0)
	{
		return;
	}

	USaveSubsystem* SaveSub = GetGameInstance()->GetSubsystem<USaveSubsystem>();
	if (SaveSub)
	{
		SaveSub->SaveToSlot(PendingOverwriteSlotIndex);
	}

	if (ConfirmOverlay)
	{
		ConfirmOverlay->SetVisibility(ESlateVisibility::Collapsed);
	}

	PendingOverwriteSlotIndex = -1;
	CloseMenu();
}

void USaveLoadMenuWidget::HandleConfirmNo()
{
	if (ConfirmOverlay)
	{
		ConfirmOverlay->SetVisibility(ESlateVisibility::Collapsed);
	}

	PendingOverwriteSlotIndex = -1;
}

void USaveLoadMenuWidget::HandleBackClicked()
{
	CloseMenu();
}

void USaveLoadMenuWidget::RefreshSlots()
{
	USaveSubsystem* SaveSub = GetGameInstance()->GetSubsystem<USaveSubsystem>();
	if (!SaveSub)
	{
		UE_LOG(LogSerene, Error, TEXT("USaveLoadMenuWidget::RefreshSlots - SaveSubsystem is null"));
		return;
	}

	TArray<USaveSlotWidget*> Slots = GetSlotWidgets();
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		if (!Slots[i])
		{
			continue;
		}

		if (SaveSub->DoesSaveExist(i))
		{
			const FSaveSlotInfo Info = SaveSub->GetSlotInfo(i);
			Slots[i]->SetSlotData(i, Info, true);
		}
		else
		{
			Slots[i]->SetSlotData(i, FSaveSlotInfo(), false);
		}
	}
}

TArray<USaveSlotWidget*> USaveLoadMenuWidget::GetSlotWidgets() const
{
	return { Slot0, Slot1, Slot2 };
}
