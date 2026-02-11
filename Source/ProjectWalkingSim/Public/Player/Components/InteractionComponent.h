// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractableChanged, AActor*, NewInteractable, FText, InteractionText);

/**
 * Handles interaction detection via camera-center line trace.
 *
 * Every tick, traces from the player's camera (view point) forward at InteractionRange
 * distance. If the hit actor implements IInteractable and CanInteract returns true,
 * it becomes the focused interactable. Focus changes fire OnFocusBegin/End on the
 * interactable and broadcast OnInteractableChanged for HUD prompt updates.
 *
 * TryInteract() is called by the PlayerController when the player presses E.
 *
 * Attach to ASereneCharacter. Requires the owning pawn to have a controller
 * with GetPlayerViewPoint() support (standard APlayerController).
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTWALKINGSIM_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	/** Attempt to interact with the currently focused interactable. Called by PlayerController on E press. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void TryInteract();

	/** Returns the actor currently being looked at, or nullptr. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	AActor* GetCurrentInteractable() const;

	/** Broadcast when the focused interactable changes (or becomes nullptr). */
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractableChanged OnInteractableChanged;

protected:
	/** Maximum distance (cm) for the interaction line trace. ~1.5m. */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	float InteractionRange = 150.0f;

	/** Collision channel used for the interaction trace. */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

private:
	/** The actor currently in focus (looked at within range). */
	TWeakObjectPtr<AActor> CurrentInteractable;

	/** Interval in seconds between interaction traces. 0.05s = 20Hz. */
	float TraceInterval = 0.05f;

	/** Accumulated time since last trace. */
	float TimeSinceLastTrace = 0.0f;

	/** Perform per-tick line trace from camera center to detect interactables. */
	void PerformTrace();

	/** Update the focused interactable, firing OnFocusBegin/End and broadcasting delegate. */
	void SetCurrentInteractable(AActor* NewInteractable);
};
