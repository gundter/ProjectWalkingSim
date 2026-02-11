// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Hiding/HidingTypes.h"
#include "HidingComponent.generated.h"

class AHidingSpotActor;
class UHidingSpotDataAsset;
class UInputMappingContext;
class UAnimMontage;

/**
 * Manages the full player hiding lifecycle as a 4-state machine.
 *
 * State flow: Free -> Entering -> Hidden -> Exiting -> Free
 *
 * EnterHidingSpot() disables movement, plays entry montage, blends camera to
 * the hiding spot's view target. Montage completion delegates drive state
 * transitions (not tick polling). ExitHidingSpot() plays exit montage, blends
 * camera back, restores movement.
 *
 * While Hidden:
 * - Look constraints are set relative to hiding spot orientation (world-space yaw offset)
 * - Input context switches to hiding-only (look + interact-to-exit, no movement)
 * - Player mesh is hidden
 * - Visibility score reduction is applied
 *
 * Attach to ASereneCharacter. Called by AHidingSpotActor::OnInteract.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTWALKINGSIM_API UHidingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHidingComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Public API ---

	/** Initiate the hiding sequence. Called by HidingSpotActor::OnInteract. */
	UFUNCTION(BlueprintCallable, Category = "Hiding")
	void EnterHidingSpot(AHidingSpotActor* Spot);

	/** Exit the current hiding spot. Called by exit input action. */
	UFUNCTION(BlueprintCallable, Category = "Hiding")
	void ExitHidingSpot();

	/** Returns the current hiding state (Free, Entering, Hidden, Exiting). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Hiding")
	EHidingState GetHidingState() const { return HidingState; }

	/** True if the player is in the Hidden state (fully hidden, not transitioning). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Hiding")
	bool IsHiding() const { return HidingState == EHidingState::Hidden; }

	/** True if the player is in the Entering or Exiting state. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Hiding")
	bool IsInTransition() const { return HidingState == EHidingState::Entering || HidingState == EHidingState::Exiting; }

	/** Returns the current hiding spot actor, or nullptr if not hiding. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Hiding")
	AHidingSpotActor* GetCurrentHidingSpot() const { return CurrentHidingSpot.Get(); }

	// --- Delegate ---

	/** Broadcast when the player's hiding state changes. */
	UPROPERTY(BlueprintAssignable, Category = "Hiding")
	FOnHidingStateChanged OnHidingStateChanged;

protected:
	// --- Input Configuration (assign in BP subclass) ---

	/** The default Input Mapping Context to remove when hiding and restore on exit. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** Input Mapping Context active while hiding (look + interact-to-exit only). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> HidingMappingContext;

	/** Priority for the hiding IMC (higher than default 0 to override). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	int32 HidingInputPriority = 1;

private:
	// --- State ---

	/** Current state of the hiding state machine. */
	EHidingState HidingState = EHidingState::Free;

	/** The hiding spot actor the player is currently in or transitioning to/from. */
	TWeakObjectPtr<AHidingSpotActor> CurrentHidingSpot;

	// --- Hiding Camera ---

	/** Base rotation of the hiding camera when entering Hidden state. */
	FRotator HidingCameraBaseRotation = FRotator::ZeroRotator;

	// --- Saved Camera Limits ---

	/** Saved camera manager ViewPitchMin to restore on exit. */
	float SavedViewPitchMin = 0.0f;

	/** Saved camera manager ViewPitchMax to restore on exit. */
	float SavedViewPitchMax = 0.0f;

	/** Saved camera manager ViewYawMin to restore on exit. */
	float SavedViewYawMin = 0.0f;

	/** Saved camera manager ViewYawMax to restore on exit. */
	float SavedViewYawMax = 0.0f;

	// --- Montage Callbacks ---

	/** Called when the entry montage finishes playing. */
	void OnEntryMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/** Called when the exit montage finishes playing. */
	void OnExitMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// --- State Transitions ---

	/** Transitions from Entering to Hidden. Called after entry montage completes. */
	void TransitionToHiddenState();

	/** Transitions from Exiting to Free. Called after exit montage completes. */
	void TransitionToFreeState();

	// --- Camera Constraints ---

	/** Save current camera limits and apply hiding spot look constraints. */
	void ApplyLookConstraints();

	/** Restore previously saved camera limits. */
	void RestoreLookConstraints();

	// --- Player System Control ---

	/** Disable movement, interaction, headbob, lean, and footstep components. */
	void DisablePlayerSystems();

	/** Re-enable all player systems disabled by DisablePlayerSystems. */
	void RestorePlayerSystems();

	// --- Mesh Visibility ---

	/** Set visibility on all skeletal mesh components of the owning character. */
	void SetPlayerMeshVisibility(bool bVisible);

	// --- Input Context ---

	/** Remove default IMC and add hiding IMC. */
	void SwitchToHidingInputContext();

	/** Remove hiding IMC and restore default IMC. */
	void RestoreDefaultInputContext();
};
