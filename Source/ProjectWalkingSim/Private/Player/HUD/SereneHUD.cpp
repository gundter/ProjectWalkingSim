// Copyright Null Lantern.

#include "Player/HUD/SereneHUD.h"

#include "Player/HUD/SereneHUDWidget.h"
#include "Player/HUD/StaminaBarWidget.h"
#include "Player/HUD/InteractionPromptWidget.h"
#include "Player/SereneCharacter.h"
#include "Player/Components/StaminaComponent.h"
#include "Player/Components/InteractionComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemDataAsset.h"
#include "Player/HUD/InventoryWidget.h"
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
	if (HUDWidgetInstance && HUDWidgetInstance->GetInventoryWidget() && CachedInventoryComp)
	{
		HUDWidgetInstance->GetInventoryWidget()->RefreshSlots(
			CachedInventoryComp->GetSlots(), CachedInventoryComp);
	}
}

void ASereneHUD::ShowInventory()
{
	if (HUDWidgetInstance && HUDWidgetInstance->GetInventoryWidget())
	{
		HUDWidgetInstance->GetInventoryWidget()->ShowInventory();
		// Do an immediate refresh so slots are current when inventory opens
		HandleInventoryChanged();
	}
}

void ASereneHUD::HideInventory()
{
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

	// Key item warning: log warning but still allow discard (per CONTEXT: "CAN be discarded with strong warning")
	if (ItemData && ItemData->bIsKeyItem)
	{
		UE_LOG(LogSerene, Warning, TEXT("ASereneHUD::HandleDiscardRequested - Discarding KEY ITEM at slot %d: %s"),
			SlotIndex, *Slots[SlotIndex].ItemId.ToString());
		// TODO: Show confirmation dialog in future iteration
	}

	CachedInventoryComp->DiscardItem(SlotIndex);
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
