// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractableBase.h"
#include "DoorActor.generated.h"

/**
 * Animated door that opens and closes on interaction.
 *
 * Root MeshComponent serves as the door frame. DoorMesh is the door panel
 * that rotates around the Z axis. The door swings toward the player on first
 * interaction (dot product check), then toggles between open and closed.
 *
 * Uses tick-based FInterpTo for smooth rotation animation.
 */
UCLASS()
class PROJECTWALKINGSIM_API ADoorActor : public AInteractableBase
{
	GENERATED_BODY()

public:
	ADoorActor();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void OnInteract_Implementation(AActor* Interactor) override;

	/** The door panel mesh, child of the root frame. Rotates on interaction. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	/** Angle in degrees the door opens to. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door")
	float OpenAngle = 90.0f;

	/** Interpolation speed for door rotation. Higher = faster. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door")
	float OpenSpeed = 3.0f;

	/** If true, the door opens away from the interacting player on first use. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door")
	bool bOpensTowardPlayer = true;

private:
	/** Whether the door is currently open. */
	bool bIsOpen = false;

	/** Current interpolated rotation angle. */
	float CurrentAngle = 0.0f;

	/** Target angle (+/- OpenAngle depending on player side, or 0 when closed). */
	float TargetAngle = 0.0f;

	/** Direction multiplier: +1 or -1 based on which side the player approached. */
	float OpenDirection = 1.0f;
};
