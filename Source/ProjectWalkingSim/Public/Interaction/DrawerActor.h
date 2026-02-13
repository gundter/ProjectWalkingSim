// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractableBase.h"
#include "Interaction/SaveableInterface.h"
#include "DrawerActor.generated.h"

/**
 * Sliding drawer/cabinet actor for The Juniper Tree.
 *
 * Root MeshComponent serves as the frame/cabinet. DrawerMesh is the sliding
 * drawer that translates along its local X axis (forward) on interaction.
 * Toggles between open and closed states with smooth interpolation.
 *
 * Uses tick-based FInterpTo for smooth slide animation.
 */
UCLASS()
class PROJECTWALKINGSIM_API ADrawerActor : public AInteractableBase, public ISaveable
{
	GENERATED_BODY()

public:
	ADrawerActor();

	virtual void Tick(float DeltaTime) override;

	// --- ISaveable ---
	virtual FName GetSaveId_Implementation() const override;
	virtual void WriteSaveData_Implementation(USereneSaveGame* SaveGame) override;
	virtual void ReadSaveData_Implementation(USereneSaveGame* SaveGame) override;

protected:
	virtual void OnInteract_Implementation(AActor* Interactor) override;

	/** The drawer mesh that slides open, child of root frame. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drawer")
	TObjectPtr<UStaticMeshComponent> DrawerMesh;

	/** Distance in cm the drawer slides out. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drawer")
	float OpenDistance = 40.0f;

	/** Interpolation speed for drawer slide. Higher = faster. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drawer")
	float OpenSpeed = 4.0f;

private:
	/** Whether the drawer is currently open. */
	bool bIsOpen = false;

	/** Current interpolated slide position. */
	float CurrentSlide = 0.0f;

	/** Initial relative location of the DrawerMesh, cached on construction. */
	FVector DrawerInitialLocation = FVector::ZeroVector;
};
