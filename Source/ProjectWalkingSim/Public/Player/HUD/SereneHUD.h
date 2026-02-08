// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SereneHUD.generated.h"

class USereneHUDWidget;
class ASereneCharacter;

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
};
