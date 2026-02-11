// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NoiseReportingComponent.generated.h"

class UFootstepComponent;

/**
 * Bridges FootstepComponent events to the AI hearing system.
 *
 * Lives on the player character (the noise source). Listens to
 * FootstepComponent::OnFootstep and reports noise events via
 * UAISense_Hearing::ReportNoiseEvent when volume exceeds the
 * sprint threshold.
 *
 * Walking (Volume=1.0) and crouching (Volume=0.3) are silent to AI.
 * Only sprinting (Volume=1.5) generates noise events.
 */
UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class PROJECTWALKINGSIM_API UNoiseReportingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNoiseReportingComponent();

protected:
	virtual void BeginPlay() override;

	// --- Tunable Properties ---

	/** Range at which sprint noise can be heard by AI (cm). Default 20m. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Noise")
	float SprintNoiseRange = 2000.0f;

	/**
	 * Footstep volume above which noise is reported to AI.
	 * Walk volume is 1.0, sprint is 1.5, crouch is 0.3.
	 * Default threshold 1.0 means only sprint footsteps generate noise.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Noise")
	float SprintVolumeThreshold = 1.0f;

private:
	/**
	 * Bound to FootstepComponent::OnFootstep.
	 * Reports noise to AI hearing when volume exceeds SprintVolumeThreshold.
	 */
	UFUNCTION()
	void HandleFootstep(EPhysicalSurface SurfaceType, float Volume);
};
