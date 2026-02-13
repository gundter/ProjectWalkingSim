// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractableBase.h"
#include "Interaction/SaveableInterface.h"
#include "DoorActor.generated.h"

/**
 * Animated door that opens and closes on interaction.
 *
 * Root MeshComponent serves as the door frame. DoorMesh is the door panel
 * that rotates around the Z axis. The door swings toward the player on first
 * interaction (dot product check), then toggles between open and closed.
 *
 * Supports optional key-lock mechanics: set RequiredItemId and bIsLocked to
 * require a specific key item to unlock. The key is consumed on unlock.
 *
 * Uses tick-based FInterpTo for smooth rotation animation.
 */
UCLASS()
class PROJECTWALKINGSIM_API ADoorActor : public AInteractableBase, public ISaveable
{
	GENERATED_BODY()

public:
	ADoorActor();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual FText GetInteractionText_Implementation() const override;

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

	// --- Lock Mechanics ---

	/** Item required to unlock this door. NAME_None = not lockable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	FName RequiredItemId = NAME_None;

	/** Whether the door is currently locked. Key is consumed on unlock. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	bool bIsLocked = false;

	/** Text shown when the door is locked. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door")
	FText LockedText;

	/** Whether the door is currently open. */
	bool bIsOpen = false;

	/** Current interpolated rotation angle. */
	float CurrentAngle = 0.0f;

	/** Target angle (+/- OpenAngle depending on player side, or 0 when closed). */
	float TargetAngle = 0.0f;

	/** Direction multiplier: +1 or -1 based on which side the player approached. */
	float OpenDirection = 1.0f;

public:
	/**
	 * Open this door for an AI actor. AI cannot open locked doors.
	 * Uses the same swing-direction logic as player interaction.
	 */
	UFUNCTION(BlueprintCallable, Category = "Door")
	void OpenForAI(AActor* AIActor);

	// --- ISaveable ---
	virtual FName GetSaveId_Implementation() const override;
	virtual void WriteSaveData_Implementation(USereneSaveGame* SaveGame) override;
	virtual void ReadSaveData_Implementation(USereneSaveGame* SaveGame) override;
};
