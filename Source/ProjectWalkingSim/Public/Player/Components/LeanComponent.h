// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LeanComponent.generated.h"

/**
 * Camera-only lean component for The Juniper Tree.
 *
 * Computes a lateral camera offset and roll angle for peeking around corners.
 * Lean direction is set by the player controller input handlers (Q hold = left,
 * E hold = right). Works both standing and crouched.
 *
 * IMPORTANT: This component does NOT directly modify the camera. It computes
 * offsets that ASereneCharacter::Tick() reads and aggregates with HeadBobComponent
 * offsets before applying to the camera.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTWALKINGSIM_API ULeanComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULeanComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// --- Public API ---

	/** Set lean left state. Called by player controller on Q hold/release. */
	UFUNCTION(BlueprintCallable, Category = "Lean")
	void SetLeanLeft(bool bLean);

	/** Set lean right state. Called by player controller on E hold/release. */
	UFUNCTION(BlueprintCallable, Category = "Lean")
	void SetLeanRight(bool bLean);

	/** True if currently leaning in either direction (alpha != 0). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lean")
	bool IsLeaning() const;

	/** Current lean alpha: -1 (full left), 0 (center), 1 (full right). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lean")
	float GetLeanAlpha() const;

	/** Returns the current lateral offset in local space. Read by character for aggregation. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lean")
	FVector GetLeanOffset() const;

	/** Returns the current camera roll in degrees. Read by character for aggregation. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lean")
	float GetLeanRoll() const;

protected:
	// --- Tuning ---

	/** Maximum lateral distance in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lean")
	float LeanDistance = 30.0f;

	/** Maximum camera roll angle in degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lean")
	float LeanRollAngle = 5.0f;

	/** Interpolation speed for smooth lean transitions. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lean")
	float LeanSpeed = 8.0f;

private:
	/** True while Q is held. */
	bool bLeaningLeft = false;

	/** True while E is held. */
	bool bLeaningRight = false;

	/** Smoothed lean state: -1 (full left), 0 (center), 1 (full right). */
	float CurrentLeanAlpha = 0.0f;
};
