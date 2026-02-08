// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SereneHUD.generated.h"

class UStaminaBarWidget;
class UInteractionPromptWidget;
class ASereneCharacter;

/**
 * HUD class for The Juniper Tree.
 *
 * Creates and manages all screen-space HUD widgets:
 * - StaminaBarWidget: progress bar bound to StaminaComponent::OnStaminaChanged
 * - InteractionPromptWidget: prompt bound to InteractionComponent::OnInteractableChanged
 *
 * Widget classes are set via EditDefaultsOnly properties so designers can
 * assign UMG Blueprint subclasses (WBP_StaminaBar, WBP_InteractionPrompt)
 * without modifying C++.
 *
 * Set as HUDClass on ASereneGameMode.
 */
UCLASS()
class PROJECTWALKINGSIM_API ASereneHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	/** UMG Blueprint class for the stamina bar widget. Assign WBP_StaminaBar here. */
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UStaminaBarWidget> StaminaBarWidgetClass;

	/** UMG Blueprint class for the interaction prompt widget. Assign WBP_InteractionPrompt here. */
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UInteractionPromptWidget> InteractionPromptWidgetClass;

private:
	/** Live instance of the stamina bar widget. */
	UPROPERTY()
	TObjectPtr<UStaminaBarWidget> StaminaBarInstance;

	/** Live instance of the interaction prompt widget. */
	UPROPERTY()
	TObjectPtr<UInteractionPromptWidget> InteractionPromptInstance;

	/**
	 * Bind HUD widgets to the character's component delegates.
	 * Called from BeginPlay once the pawn is valid.
	 */
	void BindToCharacter(ASereneCharacter* Character);

	// --- Delegate Handlers ---

	/** Forwards stamina percent changes to the StaminaBarWidget. */
	UFUNCTION()
	void HandleStaminaChanged(float Percent);

	/** Forwards interactable changes to the InteractionPromptWidget. */
	UFUNCTION()
	void HandleInteractableChanged(AActor* NewInteractable, FText InteractionText);
};
