// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SereneHUDWidget.generated.h"

class UStaminaBarWidget;
class UInteractionPromptWidget;
class UInventoryWidget;

/**
 * Root HUD container widget.
 *
 * Holds all HUD child widgets under a single Canvas Panel to reduce
 * draw overhead. Create a UMG Blueprint subclass (WBP_SereneHUD),
 * add WBP_StaminaBar and WBP_InteractionPrompt as child widgets,
 * and name them to match the BindWidget properties below.
 *
 * This class contains no logic -- children manage their own behavior.
 */
UCLASS()
class PROJECTWALKINGSIM_API USereneHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UStaminaBarWidget* GetStaminaBarWidget() const { return StaminaBarWidget; }
	UInteractionPromptWidget* GetInteractionPromptWidget() const { return InteractionPromptWidget; }
	UInventoryWidget* GetInventoryWidget() const { return InventoryWidget; }

protected:
	/** Stamina bar child widget. Name must match in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UStaminaBarWidget> StaminaBarWidget;

	/** Interaction prompt child widget. Name must match in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInteractionPromptWidget> InteractionPromptWidget;

	/** Inventory container widget. Name must match in UMG Blueprint. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryWidget> InventoryWidget;
};
