// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractableInterface.h"
#include "Interaction/HideableInterface.h"
#include "HidingSpotActor.generated.h"

class UHidingSpotDataAsset;
class UHidingComponent;
class UCameraComponent;
class UStaticMeshComponent;

/**
 * Placeable hiding spot actor (lockers, closets, under-bed spaces).
 *
 * Implements both IInteractable (player can interact via line trace) and
 * IHideable (provides hiding camera, occupancy, and discovery state).
 *
 * NOT abstract -- per-type visual differences are data-driven via
 * UHidingSpotDataAsset. Level designers place this actor, assign a mesh,
 * position the HidingCamera, and reference the appropriate data asset.
 *
 * Interaction flow:
 *   Player trace -> CanInteract -> OnInteract -> HidingComponent->EnterHidingSpot(this)
 */
UCLASS()
class PROJECTWALKINGSIM_API AHidingSpotActor : public AActor, public IInteractable, public IHideable
{
	GENERATED_BODY()

public:
	AHidingSpotActor();

protected:
	virtual void BeginPlay() override;

	// ------------------------------------------------------------------
	// Components
	// ------------------------------------------------------------------

	/** Root scene component for transform. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/**
	 * Visual mesh (locker, closet, bed frame, etc.).
	 * Assign per-instance in the editor. Serves as the interaction trace target.
	 *
	 * NOTE: A future enhancement could swap to USkeletalMeshComponent for
	 * hiding spots with animated doors (e.g. locker door swing).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SpotMesh;

	/**
	 * Camera the player views from while hidden.
	 * Position and rotation set by the level designer in the editor.
	 * bAutoActivate = false; activated via SetViewTargetWithBlend by HidingComponent.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> HidingCamera;

	// ------------------------------------------------------------------
	// Configuration
	// ------------------------------------------------------------------

	/**
	 * Per-type hiding spot configuration (montages, camera limits, peek material).
	 * Assign in the editor per-instance or via data asset reference.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hiding")
	TObjectPtr<UHidingSpotDataAsset> SpotData;

	// ------------------------------------------------------------------
	// State
	// ------------------------------------------------------------------

	/** Whether someone is currently hiding in this spot. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hiding")
	bool bIsOccupied = false;

	/** Persists with save system -- true if the monster previously found a player here. */
	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "Hiding")
	bool bDiscoveredByMonster = false;

	/** Weak reference to the actor currently hiding (nullptr if unoccupied). */
	TWeakObjectPtr<AActor> OccupantActor;

	// ------------------------------------------------------------------
	// IInteractable Implementation
	// ------------------------------------------------------------------

	virtual FText GetInteractionText_Implementation() const override;
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual void OnFocusBegin_Implementation(AActor* Interactor) override;
	virtual void OnFocusEnd_Implementation(AActor* Interactor) override;

	// ------------------------------------------------------------------
	// IHideable Implementation
	// ------------------------------------------------------------------

	virtual bool CanHide_Implementation(AActor* HidingActor) const override;
	virtual void OnEnterHiding_Implementation(AActor* HidingActor) override;
	virtual void OnExitHiding_Implementation(AActor* HidingActor) override;
	virtual UCameraComponent* GetHidingCamera_Implementation() const override;
	virtual UHidingSpotDataAsset* GetSpotData_Implementation() const override;
	virtual bool IsOccupied_Implementation() const override;
	virtual void MarkDiscovered_Implementation() override;
	virtual bool WasDiscovered_Implementation() const override;
};
