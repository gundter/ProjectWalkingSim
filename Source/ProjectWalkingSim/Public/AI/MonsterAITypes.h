// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "MonsterAITypes.generated.h"

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
 * Behavioral state of the Wendigo.
 * More granular than EAlertLevel -- tracks what the Wendigo is *doing*,
 * not just its perception-driven alert state.
 */
UENUM(BlueprintType)
enum class EWendigoBehaviorState : uint8
{
	Patrol        UMETA(DisplayName = "Patrol"),
	Investigating UMETA(DisplayName = "Investigating"),
	Chasing       UMETA(DisplayName = "Chasing"),
	Searching     UMETA(DisplayName = "Searching"),
	GrabAttack    UMETA(DisplayName = "Grab Attack")
};

/** Broadcast when the Wendigo's behavior state changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBehaviorStateChanged, EWendigoBehaviorState, NewState);

/**
 * Type of stimulus that triggered the current suspicion/investigation.
 * Used to differentiate investigation speed and behavior.
 */
UENUM(BlueprintType)
enum class EStimulusType : uint8
{
	None  UMETA(DisplayName = "None"),
	Sound UMETA(DisplayName = "Sound"),
	Sight UMETA(DisplayName = "Sight")
};

/**
 * AI tuning constants.
 * Centralized defaults for perception, suspicion, and movement parameters.
 * Individual components may override these via UPROPERTY editable values.
 */
namespace AIConstants
{
	/** Player visibility score below this is invisible to AI. */
	constexpr float VisibilityThreshold = 0.05f;

	/** Base suspicion gain per second at full visibility. */
	constexpr float BaseSuspicionRate = 1.5f;

	/** Suspicion decay per second when player not visible (~15s from full). */
	constexpr float SuspicionDecayRate = 0.065f;

	/** Suspicion value that triggers Suspicious alert level. */
	constexpr float SuspiciousThreshold = 0.2f;

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

	/** Wendigo chase speed in cm/s (~15% faster than player sprint 500). */
	constexpr float WendigoChaseSpeed = 575.0f;

	/** Wendigo search speed in cm/s (between patrol 150 and investigate 200). */
	constexpr float WendigoSearchSpeed = 180.0f;

	/** Wendigo investigation speed for visual stimulus in cm/s (faster than hearing). */
	constexpr float WendigoInvestigateSightSpeed = 250.0f;

	/** Seconds of lost line-of-sight before chase transitions to search. */
	constexpr float LOSLostTimeout = 3.0f;

	/** Distance in cm for grab attack (~1.5m). */
	constexpr float GrabRange = 150.0f;

	/** Radius in cm for random search points around last-known location. */
	constexpr float SearchRadius = 600.0f;

	/** Total search duration in seconds before returning to patrol. */
	constexpr float SearchDuration = 18.0f;

	/** Number of random navmesh points to visit during search. */
	constexpr int32 NumSearchPoints = 3;
}
