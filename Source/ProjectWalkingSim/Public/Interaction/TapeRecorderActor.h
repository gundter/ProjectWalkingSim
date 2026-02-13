// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractableBase.h"
#include "TapeRecorderActor.generated.h"

class USaveLoadMenuWidget;

/**
 * World-placed tape recorder that serves as a save point.
 *
 * The detective records case notes on the tape recorder, which opens
 * the save slot picker (SaveLoadMenuWidget in Save mode). The player's
 * saved position is set to the tape recorder location so they respawn here.
 *
 * Level designers place instances and assign SaveMenuWidgetClass in the
 * BP subclass (reparent to ATapeRecorderActor).
 */
UCLASS()
class PROJECTWALKINGSIM_API ATapeRecorderActor : public AInteractableBase
{
	GENERATED_BODY()

public:
	ATapeRecorderActor();

	// --- IInteractable Overrides ---

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual FText GetInteractionText_Implementation() const override;
	virtual void OnInteract_Implementation(AActor* Interactor) override;

protected:
	/** Sound played when the player starts recording (tape rolling). Assigned in Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Save")
	TObjectPtr<USoundBase> SaveSound;

	/** Duration of save feedback before the slot picker opens. Can be 0 to skip. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Save")
	float SaveAnimDuration = 1.5f;

	/** Blueprint subclass of SaveLoadMenuWidget. Assigned in BP subclass. */
	UPROPERTY(EditDefaultsOnly, Category = "Save")
	TSubclassOf<USaveLoadMenuWidget> SaveMenuWidgetClass;

private:
	/** Live save menu widget instance. */
	UPROPERTY()
	TObjectPtr<USaveLoadMenuWidget> SaveMenuInstance;

	/** Called when the save menu is closed. Restores input mode and cleans up. */
	UFUNCTION()
	void HandleSaveMenuClosed();
};
