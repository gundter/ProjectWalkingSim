// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "AITypes.generated.h"

/**
 * Alert level for AI-controlled characters.
 * Drives State Tree transitions and behavior selection.
 */
UENUM(BlueprintType)
enum class EAlertLevel : uint8
{
	Patrol     UMETA(DisplayName = "Patrol"),
	Suspicious UMETA(DisplayName = "Suspicious"),
	Alert      UMETA(DisplayName = "Alert")
};

/** Broadcast when the AI's alert level changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAlertLevelChanged, EAlertLevel, NewLevel);

/**
 * AI tuning constants.
 * Centralized defaults for perception, suspicion, and movement parameters.
 * Individual components may override these via UPROPERTY editable values.
 */
namespace AIConstants
{
	/** Player visibility score below this is invisible to AI. */
	constexpr float VisibilityThreshold = 0.3f;

	/** Base suspicion gain per second at full visibility. */
	constexpr float BaseSuspicionRate = 0.15f;

	/** Suspicion decay per second when player not visible (~15s from full). */
	constexpr float SuspicionDecayRate = 0.065f;

	/** Suspicion value that triggers Suspicious alert level. */
	constexpr float SuspiciousThreshold = 0.4f;

	/** Suspicion value that triggers Alert alert level. */
	constexpr float AlertThreshold = 0.8f;

	/** Wendigo default walk speed in cm/s (slower than player's 250). */
	constexpr float WendigoWalkSpeed = 150.0f;

	/** Maximum sight detection range in cm (~25m). */
	constexpr float SightRange = 2500.0f;

	/** Range at which AI loses sight of a detected target in cm (~30m). */
	constexpr float LoseSightRange = 3000.0f;

	/** Half-angle of the sight cone in degrees (90 total FOV). */
	constexpr float SightHalfAngle = 45.0f;

	/** Maximum hearing detection range in cm (~20m). */
	constexpr float HearingRange = 2000.0f;

	/** Range at which sprint footsteps generate noise events in cm. */
	constexpr float SprintNoiseRange = 2000.0f;
}
