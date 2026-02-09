// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SerenePlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class ASereneCharacter;
class ASereneHUD;
struct FInputActionValue;

/**
 * Player controller for The Juniper Tree.
 *
 * Manages Enhanced Input binding and dispatches input to ASereneCharacter.
 * All input actions (Move, Look, Sprint, Crouch, Interact, Lean) are data-driven
 * via UInputAction and UInputMappingContext assets.
 *
 * Assign IA_ and IMC_ assets in the BP subclass or via DefaultObject.
 * The actual key bindings (WASD, Mouse, Shift, Ctrl, E, Q) are configured
 * in those data assets in Content/Input/, not in C++.
 *
 * Note on E key: InteractAction (E press/tap) and LeanRightAction (E hold) share
 * the E key. They are differentiated by Enhanced Input triggers (Pressed vs Hold
 * with 0.2s threshold). This is configured in the IMC data asset, not in C++.
 */
UCLASS()
class PROJECTWALKINGSIM_API ASerenePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// --- Input Assets (assign in BP subclass or DefaultObject) ---

	/** Default mapping context added at priority 0 on BeginPlay. */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** Axis2D: WASD directional movement. */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	/** Axis2D: Mouse look (yaw/pitch). */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	/** Bool: Shift hold for sprint. */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> SprintAction;

	/** Bool: Ctrl for crouch (toggle by default, hold via setting). */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> CrouchAction;

	/** Bool: E tap for interact. */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	/** Bool: Q hold for lean left. */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LeanLeftAction;

	/** Bool: E hold for lean right (differentiated from Interact by hold threshold). */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LeanRightAction;

	/** Bool: Tab (or assigned key) to toggle inventory. */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> ToggleInventoryAction;

private:
	// --- Input Handlers ---

	void HandleMove(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void HandleSprintStart(const FInputActionValue& Value);
	void HandleSprintStop(const FInputActionValue& Value);
	void HandleCrouchToggle(const FInputActionValue& Value);
	void HandleInteract(const FInputActionValue& Value);
	void HandleLeanLeftStart(const FInputActionValue& Value);
	void HandleLeanLeftStop(const FInputActionValue& Value);
	void HandleLeanRightStart(const FInputActionValue& Value);
	void HandleLeanRightStop(const FInputActionValue& Value);

	/** Cached pointer to the controlled SereneCharacter. Updated on Possess. */
	ASereneCharacter* GetSereneCharacter() const;

	// --- Inventory Toggle ---

	/** Whether the inventory UI is currently open. */
	bool bIsInventoryOpen = false;

	void HandleToggleInventory(const FInputActionValue& Value);
	void OpenInventory();
	void CloseInventory();
};
