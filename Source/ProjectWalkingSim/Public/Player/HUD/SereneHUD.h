// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SereneHUD.generated.h"

class USereneHUDWidget;
class ASereneCharacter;
class UInventoryComponent;

/**
 * HUD class for The Juniper Tree.
 *
 * Creates a single root widget (SereneHUDWidget) containing all HUD children
 * under one Canvas Panel, reducing draw overhead versus separate viewport widgets.
 *
 * Widget class is set via EditDefaultsOnly so designers can assign the UMG
 * Blueprint subclass (WBP_SereneHUD) without modifying C++.
 *
 * Set as HUDClass on ASereneGameMode.
 */
UCLASS()
class PROJECTWALKINGSIM_API ASereneHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	/** UMG Blueprint class for the root HUD widget. Assign WBP_SereneHUD here. */
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<USereneHUDWidget> HUDWidgetClass;

public:
	/** Bind HUD widgets to the character's component delegates. Called from PossessedBy. */
	void BindToCharacter(ASereneCharacter* Character);

	/** Show the inventory UI. Called by PlayerController. */
	void ShowInventory();

	/** Hide the inventory UI. Called by PlayerController. */
	void HideInventory();

private:
	/** Live instance of the root HUD widget. */
	UPROPERTY()
	TObjectPtr<USereneHUDWidget> HUDWidgetInstance;

	// --- Delegate Handlers ---

	/** Forwards stamina percent changes to the StaminaBarWidget. */
	UFUNCTION()
	void HandleStaminaChanged(float Percent);

	/** Forwards interactable changes to the InteractionPromptWidget. */
	UFUNCTION()
	void HandleInteractableChanged(AActor* NewInteractable, FText InteractionText);

	/** Forwards inventory changes to the InventoryWidget for refresh. */
	UFUNCTION()
	void HandleInventoryChanged();

	/** Handles tooltip Use action from InventoryWidget. */
	UFUNCTION()
	void HandleUseRequested(int32 SlotIndex);

	/** Handles tooltip Discard action from InventoryWidget. */
	UFUNCTION()
	void HandleDiscardRequested(int32 SlotIndex);

	/** Handles combine button click from tooltip: enters combine mode on InventoryWidget. */
	UFUNCTION()
	void HandleCombineButtonClicked(int32 SlotIndex);

	/** Handles combine slot selection from InventoryWidget: calls TryCombineItems. */
	UFUNCTION()
	void HandleCombineSlotSelected(int32 SlotIndexA, int32 SlotIndexB);

	/** Cached pointer to the character's inventory component. Set in BindToCharacter. */
	UPROPERTY()
	TObjectPtr<UInventoryComponent> CachedInventoryComp;

	/** Slot index pending discard confirmation for key items. -1 if none. */
	int32 PendingDiscardSlotIndex = -1;

	/** Clear any pending discard confirmation state. */
	void ClearPendingDiscard();
};
