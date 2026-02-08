// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HeadBobComponent.generated.h"

/**
 * Procedural sine-wave head-bob component for The Juniper Tree.
 *
 * Computes a camera offset based on the owning character's movement speed,
 * using frame-rate-independent DeltaTime accumulation for the sine wave.
 * Scales amplitude for sprint and crouch states. Can be toggled off for
 * accessibility (GameInstance bHeadBobEnabled setting).
 *
 * IMPORTANT: This component does NOT directly modify the camera. It computes
 * offsets that ASereneCharacter::Tick() reads and aggregates with LeanComponent
 * offsets before applying to the camera.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTWALKINGSIM_API UHeadBobComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHeadBobComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// --- Public API ---

	/** Enable or disable head-bob. When disabled, offset smoothly fades to zero. */
	UFUNCTION(BlueprintCallable, Category = "Head Bob")
	void SetEnabled(bool bEnable);

	/** Whether head-bob is currently enabled. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Head Bob")
	bool IsEnabled() const;

	/** Returns the current computed camera offset (local space). Read by character for aggregation. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Head Bob")
	FVector GetCurrentOffset() const;

protected:
	// --- Tuning ---

	/** Vertical displacement in cm at walk speed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Head Bob")
	float WalkBobAmplitude = 0.4f;

	/** Oscillations per second at walk speed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Head Bob")
	float WalkBobFrequency = 6.0f;

	/** Amplitude multiplier when the character is sprinting. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Head Bob")
	float SprintBobMultiplier = 1.5f;

	/** Amplitude multiplier when the character is crouching. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Head Bob")
	float CrouchBobMultiplier = 0.5f;

	/** Horizontal sway as fraction of vertical amplitude. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Head Bob")
	float HorizontalBobRatio = 0.3f;

	/** How quickly bob fades in (when moving) or out (when stopped). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Head Bob")
	float BobInterpSpeed = 6.0f;

private:
	/** Whether the component is enabled. Controlled by accessibility setting. */
	bool bEnabled = true;

	/** Accumulated time for sine wave. Frame-rate independent via DeltaTime. */
	float BobTimer = 0.0f;

	/** Blend alpha: 0 when stationary, 1 when moving. Smoothly interpolated. */
	float CurrentBobAlpha = 0.0f;

	/** The computed camera offset in local space. */
	FVector CurrentBobOffset = FVector::ZeroVector;
};
