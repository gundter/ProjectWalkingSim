// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Contract for any actor the player can interact with.
 *
 * Implemented by doors, pickups, readables, drawers, and any future
 * interactable object. The InteractionComponent on the player drives
 * focus and interaction calls through this interface.
 */
class PROJECTWALKINGSIM_API IInteractable
{
	GENERATED_BODY()

public:
	/** Return the text shown in the interaction prompt (e.g., "Open", "Pick Up", "Read"). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FText GetInteractionText() const;

	/** Whether interaction is currently available for the given interactor. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract(AActor* Interactor) const;

	/** Perform the interaction. Called when the player presses the interact key. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnInteract(AActor* Interactor);

	/** Called when the player's reticle enters this interactable. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnFocusBegin(AActor* Interactor);

	/** Called when the player's reticle exits this interactable. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnFocusEnd(AActor* Interactor);
};
