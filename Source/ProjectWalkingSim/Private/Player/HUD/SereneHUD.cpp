// Copyright Null Lantern.

#include "Player/HUD/SereneHUD.h"

#include "Player/HUD/SereneHUDWidget.h"
#include "Player/HUD/StaminaBarWidget.h"
#include "Player/HUD/InteractionPromptWidget.h"
#include "Player/HUD/InventoryWidget.h"
#include "Player/HUD/ItemTooltipWidget.h"
#include "Player/SereneCharacter.h"
#include "Player/Components/StaminaComponent.h"
#include "Player/Components/InteractionComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemDataAsset.h"
#include "Core/SereneLogChannels.h"

void ASereneHUD::BeginPlay()
{
	Super::BeginPlay();

	if (HUDWidgetClass)
	{
		HUDWidgetInstance = CreateWidget<USereneHUDWidget>(GetOwningPlayerController(), HUDWidgetClass);
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport();
			UE_LOG(LogSerene, Log, TEXT("ASereneHUD::BeginPlay - HUDWidget created and added to viewport."));

			// Bind InventoryWidget action delegates
			if (HUDWidgetInstance->GetInventoryWidget())
			{
				UInventoryWidget* InvWidget = HUDWidgetInstance->GetInventoryWidget();
				InvWidget->OnUseRequested.AddDynamic(this, &ASereneHUD::HandleUseRequested);
				InvWidget->OnDiscardRequested.AddDynamic(this, &ASereneHUD::HandleDiscardRequested);
				InvWidget->OnCombineRequested.AddDynamic(this, &ASereneHUD::HandleCombineButtonClicked);
				InvWidget->OnCombineSlotSelected.AddDynamic(this, &ASereneHUD::HandleCombineSlotSelected);
			}
		}
	}
	else
	{
		UE_LOG(LogSerene, Warning, TEXT("ASereneHUD::BeginPlay - HUDWidgetClass not set. HUD will not display."));
	}
}

void ASereneHUD::BindToCharacter(ASereneCharacter* Character)
{
	if (!Character)
	{
		return;
	}

	// Bind stamina delegate to HUD handler.
	if (UStaminaComponent* Stamina = Character->FindComponentByClass<UStaminaComponent>())
	{
		Stamina->OnStaminaChanged.AddDynamic(this, &ASereneHUD::HandleStaminaChanged);
		UE_LOG(LogSerene, Log, TEXT("ASereneHUD::BindToCharacter - Bound to StaminaComponent::OnStaminaChanged."));
	}

	// Bind interaction delegate to HUD handler.
	if (UInteractionComponent* Interaction = Character->FindComponentByClass<UInteractionComponent>())
	{
		Interaction->OnInteractableChanged.AddDynamic(this, &ASereneHUD::HandleInteractableChanged);
		UE_LOG(LogSerene, Log, TEXT("ASereneHUD::BindToCharacter - Bound to InteractionComponent::OnInteractableChanged."));
	}

	// Bind inventory delegate to HUD handler.
	if (UInventoryComponent* Inventory = Character->FindComponentByClass<UInventoryComponent>())
	{
		CachedInventoryComp = Inventory;
		Inventory->OnInventoryChanged.AddDynamic(this, &ASereneHUD::HandleInventoryChanged);
		UE_LOG(LogSerene, Log, TEXT("ASereneHUD::BindToCharacter - Bound to InventoryComponent::OnInventoryChanged."));
	}
}

void ASereneHUD::HandleStaminaChanged(float Percent)
{
	if (HUDWidgetInstance && HUDWidgetInstance->GetStaminaBarWidget())
	{
		HUDWidgetInstance->GetStaminaBarWidget()->SetStaminaPercent(Percent);
	}
	else
	{
		UE_LOG(LogSerene, Warning, TEXT("ASereneHUD::HandleStaminaChanged - HUDWidgetInstance or StaminaBarWidget is null."));
	}
}

void ASereneHUD::HandleInteractableChanged(AActor* NewInteractable, FText InteractionText)
{
	if (HUDWidgetInstance && HUDWidgetInstance->GetInteractionPromptWidget())
	{
		HUDWidgetInstance->GetInteractionPromptWidget()->UpdatePrompt(NewInteractable, InteractionText);
	}
}

void ASereneHUD::HandleInventoryChanged()
{
	UE_LOG(LogSerene, Verbose, TEXT("ASereneHUD::HandleInventoryChanged - HUDWidgetInstance=%s, GetInventoryWidget=%s, CachedInventoryComp=%s"),
		HUDWidgetInstance ? TEXT("valid") : TEXT("null"),
		(HUDWidgetInstance && HUDWidgetInstance->GetInventoryWidget()) ? TEXT("valid") : TEXT("null"),
		CachedInventoryComp ? TEXT("valid") : TEXT("null"));

	if (HUDWidgetInstance && HUDWidgetInstance->GetInventoryWidget() && CachedInventoryComp)
	{
		HUDWidgetInstance->GetInventoryWidget()->RefreshSlots(
			CachedInventoryComp->GetSlots(), CachedInventoryComp);
	}
}

void ASereneHUD::ShowInventory()
{
	UE_LOG(LogSerene, Log, TEXT("ASereneHUD::ShowInventory called"));

	if (HUDWidgetInstance && HUDWidgetInstance->GetInventoryWidget())
	{
		HUDWidgetInstance->GetInventoryWidget()->ShowInventory();
		// Do an immediate refresh so slots are current when inventory opens
		HandleInventoryChanged();
	}
}

void ASereneHUD::HideInventory()
{
	ClearPendingDiscard();

	if (HUDWidgetInstance && HUDWidgetInstance->GetInventoryWidget())
	{
		HUDWidgetInstance->GetInventoryWidget()->HideInventory();
	}
}

void ASereneHUD::HandleUseRequested(int32 SlotIndex)
{
	if (!CachedInventoryComp) return;
	const TArray<FInventorySlot>& Slots = CachedInventoryComp->GetSlots();
	if (!Slots.IsValidIndex(SlotIndex) || Slots[SlotIndex].IsEmpty()) return;

	const UItemDataAsset* ItemData = CachedInventoryComp->GetItemData(Slots[SlotIndex].ItemId);
	if (!ItemData) return;

	// For now, "Use" is a placeholder. Key items don't have a direct "use" action
	// outside of door interaction. Log and potentially expand in future phases.
	UE_LOG(LogSerene, Log, TEXT("ASereneHUD::HandleUseRequested - Use requested for slot %d: %s"),
		SlotIndex, *Slots[SlotIndex].ItemId.ToString());
}

void ASereneHUD::HandleDiscardRequested(int32 SlotIndex)
{
	if (!CachedInventoryComp) return;
	const TArray<FInventorySlot>& Slots = CachedInventoryComp->GetSlots();
	if (!Slots.IsValidIndex(SlotIndex) || Slots[SlotIndex].IsEmpty()) return;

	const UItemDataAsset* ItemData = CachedInventoryComp->GetItemData(Slots[SlotIndex].ItemId);

	// Key item confirmation flow: require two clicks to discard
	if (ItemData && ItemData->bIsKeyItem)
	{
		// Check if this is a confirmation click (same slot was already pending)
		if (PendingDiscardSlotIndex == SlotIndex)
		{
			// Second click - confirmed, proceed with discard
			UE_LOG(LogSerene, Warning, TEXT("ASereneHUD::HandleDiscardRequested - Confirmed discard of KEY ITEM at slot %d: %s"),
				SlotIndex, *Slots[SlotIndex].ItemId.ToString());
			ClearPendingDiscard();
			CachedInventoryComp->DiscardItem(SlotIndex);
			return;
		}

		// First click on key item - enter confirmation mode
		PendingDiscardSlotIndex = SlotIndex;
		UE_LOG(LogSerene, Log, TEXT("ASereneHUD::HandleDiscardRequested - Key item discard pending confirmation for slot %d"), SlotIndex);

		// Update tooltip to show warning
		if (HUDWidgetInstance && HUDWidgetInstance->GetInventoryWidget())
		{
			HUDWidgetInstance->GetInventoryWidget()->SetTooltipDiscardConfirmMode(true, ItemData);
		}

		return;
	}

	// Non-key item - discard immediately
	ClearPendingDiscard();
	CachedInventoryComp->DiscardItem(SlotIndex);
}

void ASereneHUD::ClearPendingDiscard()
{
	if (PendingDiscardSlotIndex >= 0)
	{
		// Restore tooltip to normal state
		if (HUDWidgetInstance && HUDWidgetInstance->GetInventoryWidget() && CachedInventoryComp)
		{
			const TArray<FInventorySlot>& Slots = CachedInventoryComp->GetSlots();
			if (Slots.IsValidIndex(PendingDiscardSlotIndex) && !Slots[PendingDiscardSlotIndex].IsEmpty())
			{
				const UItemDataAsset* ItemData = CachedInventoryComp->GetItemData(Slots[PendingDiscardSlotIndex].ItemId);
				HUDWidgetInstance->GetInventoryWidget()->SetTooltipDiscardConfirmMode(false, ItemData);
			}
		}
		PendingDiscardSlotIndex = -1;
	}
}

void ASereneHUD::HandleCombineButtonClicked(int32 SlotIndex)
{
	if (HUDWidgetInstance && HUDWidgetInstance->GetInventoryWidget())
	{
		HUDWidgetInstance->GetInventoryWidget()->EnterCombineMode(SlotIndex);
	}
}

void ASereneHUD::HandleCombineSlotSelected(int32 SlotIndexA, int32 SlotIndexB)
{
	if (!CachedInventoryComp)
	{
		return;
	}

	CachedInventoryComp->TryCombineItems(SlotIndexA, SlotIndexB);
	// Inventory change delegate will auto-refresh the UI
}
