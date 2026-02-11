// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Math/Float16Color.h"
#include "VisibilityScoreComponent.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

/**
 * Samples ambient light around the player and outputs a 0.0-1.0 visibility score.
 *
 * Uses a SceneCaptureComponent2D rendering to a tiny (8x8) HDR render target
 * on a timer (default 0.25s). The captured pixels are read back, averaged into
 * a luminance value, then modified by crouch and hiding states.
 *
 * AI perception (Phase 4) reads GetVisibilityScore() to decide if the player
 * can be seen. Darkness becomes a real gameplay tool.
 *
 * The component is always active on ASereneCharacter. HidingComponent calls
 * SetHidingReduction() when the player enters/exits hiding spots.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTWALKINGSIM_API UVisibilityScoreComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVisibilityScoreComponent();

	// --- Public API ---

	/** Returns the current visibility score (0.0 = invisible, 1.0 = fully visible). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Visibility")
	float GetVisibilityScore() const { return VisibilityScore; }

	/** Returns the raw light level before crouch/hiding modifiers (for debug). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Visibility")
	float GetRawLightLevel() const { return RawLightLevel; }

	/**
	 * Called by HidingComponent when entering/exiting hiding spots.
	 * @param Reduction - Visibility reduction amount (0.0 = not hiding, typically 0.5 for standard spots).
	 */
	UFUNCTION(BlueprintCallable, Category = "Visibility")
	void SetHidingReduction(float Reduction);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// --- Configuration ---

	/** Seconds between light samples. Lower = more responsive but more expensive. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visibility|Capture")
	float CaptureInterval = 0.25f;

	/** Pixels per side of the capture render target (8x8 = 64 pixels). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visibility|Capture", meta=(ClampMin="2", ClampMax="32"))
	int32 CaptureResolution = 8;

	/** Luminance value that maps to 1.0 visibility. Tune per scene. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visibility|Scoring", meta=(ClampMin="0.01"))
	float MaxExpectedLuminance = 3.0f;

	/** Visibility reduction while crouching (smaller silhouette). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visibility|Scoring", meta=(ClampMin="0.0", ClampMax="1.0"))
	float CrouchReduction = 0.15f;

	/** Default hiding spot visibility reduction (can be overridden per spot). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visibility|Scoring", meta=(ClampMin="0.0", ClampMax="1.0"))
	float DefaultHidingReduction = 0.5f;

private:
	// --- Scene Capture ---

	/** SceneCaptureComponent2D created at runtime, attached to owner. */
	UPROPERTY()
	TObjectPtr<USceneCaptureComponent2D> SceneCapture;

	/** Tiny HDR render target for light sampling. */
	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	/** Timer handle for periodic capture. */
	FTimerHandle CaptureTimerHandle;

	/** Cached pixel readback buffer to avoid per-call heap allocation (CRIT-01). */
	TArray<FFloat16Color> CachedPixels;

	// --- Score State ---

	/** Current computed visibility score (0.0-1.0). */
	float VisibilityScore = 1.0f;

	/** Raw average luminance before modifiers. */
	float RawLightLevel = 1.0f;

	/** Current hiding reduction set by HidingComponent (0.0 when not hiding). */
	float CurrentHidingReduction = 0.0f;

	// --- Internal Methods ---

	/** Timer callback: triggers a scene capture and recomputes score. */
	void PerformCapture();

	/** Reads pixels from render target and computes the visibility score. */
	void ComputeScore();
};
