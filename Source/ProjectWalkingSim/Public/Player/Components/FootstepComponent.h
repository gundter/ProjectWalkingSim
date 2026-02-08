// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "FootstepComponent.generated.h"

/**
 * Delegate broadcast each time a footstep occurs.
 *
 * @param SurfaceType  The physical surface type detected beneath the foot.
 * @param Volume       Normalized volume (0-1+). Walk=1.0, sprint=higher, crouch=lower.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFootstep, EPhysicalSurface, SurfaceType, float, Volume);

/**
 * Footstep surface detection component for The Juniper Tree.
 *
 * Detects the physical material beneath the character's feet via a downward
 * line trace and broadcasts the surface type and volume for audio and AI
 * systems to consume.
 *
 * Timer-based footstep rate is the default mode (no animation assets needed).
 * When proper animations are configured, set bUseAnimNotify=true and call
 * PlayFootstepForSurface() from an AnimNotify instead -- the timer is then
 * disabled.
 *
 * Actual sound playback is deferred to Phase 6 (Audio). This component
 * detects surfaces and broadcasts events. Phase 6 will listen to OnFootstep
 * and play appropriate sounds. Phase 4/5 AI will listen to OnFootstep for
 * noise detection.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTWALKINGSIM_API UFootstepComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFootstepComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// --- Public API ---

	/**
	 * Perform a downward trace from FootLocation and broadcast the detected
	 * surface type. Can be called from AnimNotify or from the internal timer.
	 */
	UFUNCTION(BlueprintCallable, Category = "Footsteps")
	void PlayFootstepForSurface(const FVector& FootLocation);

	/** Returns the last detected physical surface type. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Footsteps")
	EPhysicalSurface GetLastSurfaceType() const { return LastSurfaceType; }

	// --- Delegate ---

	/** Broadcast each footstep with the detected surface type and volume. */
	UPROPERTY(BlueprintAssignable, Category = "Footsteps")
	FOnFootstep OnFootstep;

protected:
	// --- Tuning ---

	/** Distance (cm) to trace downward from foot location. */
	UPROPERTY(EditAnywhere, Category = "Footsteps")
	float TraceDistance = 50.0f;

	/** Seconds between footsteps at walk speed. */
	UPROPERTY(EditAnywhere, Category = "Footsteps")
	float FootstepInterval = 0.5f;

	/** Multiplier applied to interval when sprinting (faster footsteps). */
	UPROPERTY(EditAnywhere, Category = "Footsteps")
	float SprintIntervalMultiplier = 0.6f;

	/** Multiplier applied to interval when crouching (slower footsteps). */
	UPROPERTY(EditAnywhere, Category = "Footsteps")
	float CrouchIntervalMultiplier = 1.3f;

	/** Volume multiplier when crouching (quieter for future AI noise). */
	UPROPERTY(EditAnywhere, Category = "Footsteps")
	float CrouchVolumeMultiplier = 0.3f;

	/** Volume multiplier when sprinting (louder for future AI noise). */
	UPROPERTY(EditAnywhere, Category = "Footsteps")
	float SprintVolumeMultiplier = 1.5f;

	/**
	 * When true, the internal timer is disabled. Footsteps must be triggered
	 * externally via PlayFootstepForSurface() from an AnimNotify.
	 */
	UPROPERTY(EditAnywhere, Category = "Footsteps")
	bool bUseAnimNotify = false;

private:
	/** Last detected surface type from the downward trace. */
	EPhysicalSurface LastSurfaceType = EPhysicalSurface::SurfaceType_Default;

	/** Accumulates time for the timer-based footstep trigger. */
	float FootstepTimer = 0.0f;
};
