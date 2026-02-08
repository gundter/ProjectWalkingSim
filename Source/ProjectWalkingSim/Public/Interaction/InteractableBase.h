// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractableInterface.h"
#include "GameplayTagContainer.h"
#include "InteractableBase.generated.h"

/**
 * Base actor for all interactable objects in The Juniper Tree.
 *
 * Implements IInteractable with default behavior that subclasses can override.
 * Provides common properties: interaction text, gameplay tag for type identification,
 * a root static mesh, and an enable/disable flag.
 *
 * Subclasses:
 *   - ADoorActor: animated door open/close
 *   - APickupActor: generic pickup (Phase 2 inventory integration)
 *   - AReadableActor: notes and documents
 *   - ADrawerActor: sliding drawer/cabinet
 *
 * Tick is disabled by default. Subclasses that need animation (door, drawer)
 * opt in by setting PrimaryActorTick.bCanEverTick = true in their constructors.
 */
UCLASS(Abstract)
class PROJECTWALKINGSIM_API AInteractableBase : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	AInteractableBase();

	// --- IInteractable Implementation (virtual, overridable by subclasses) ---

	virtual FText GetInteractionText_Implementation() const override;
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual void OnFocusBegin_Implementation(AActor* Interactor) override;
	virtual void OnFocusEnd_Implementation(AActor* Interactor) override;

protected:
	/** Text displayed in the interaction prompt (e.g., "Open", "Pick Up", "Read"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText InteractionText;

	/** Gameplay tag identifying the interaction type (e.g., Interaction.Door). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FGameplayTag InteractionTag;

	/** Root mesh component for visual representation. Subclasses may add additional meshes. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	/** Whether this actor can currently be interacted with. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bCanBeInteracted = true;
};
