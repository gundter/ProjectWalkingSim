// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AI/MonsterAITypes.h"
#include "SuspicionComponent.generated.h"

/**
 * Tracks suspicion level for AI-controlled characters.
 * Accumulates suspicion from sight (scaled by player visibility score) and hearing stimuli.
 * Decays suspicion over time when no stimulus is present.
 * Transitions through three alert levels: Patrol -> Suspicious -> Alert.
 */
UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class PROJECTWALKINGSIM_API USuspicionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USuspicionComponent();

	// --- Public API ---

	/**
	 * Process a sight stimulus from the perception system.
	 * Accumulates suspicion scaled by the player's visibility score.
	 * @param PlayerVisibilityScore  0-1 visibility score from VisibilityScoreComponent.
	 * @param DeltaTime              Frame delta time.
	 */
	void ProcessSightStimulus(float PlayerVisibilityScore, float DeltaTime);

	/**
	 * Process a hearing stimulus (e.g., sprint footstep).
	 * Applies a fixed suspicion bump and records the stimulus location.
	 * @param StimulusLocation  World location of the noise event.
	 */
	void ProcessHearingStimulus(const FVector& StimulusLocation);

	/**
	 * Decay suspicion over time when no stimulus is present.
	 * Should be called each tick when the AI has no active sight/hearing stimulus.
	 * @param DeltaTime  Frame delta time.
	 */
	void DecaySuspicion(float DeltaTime);

	/** Get the current alert level. */
	UFUNCTION(BlueprintCallable, Category = "AI|Suspicion")
	EAlertLevel GetAlertLevel() const { return CurrentAlertLevel; }

	/** Get the raw suspicion value (0.0 to 1.0). */
	UFUNCTION(BlueprintCallable, Category = "AI|Suspicion")
	float GetCurrentSuspicion() const { return CurrentSuspicion; }

	/** Get the last known location of a stimulus (sight or hearing). */
	UFUNCTION(BlueprintCallable, Category = "AI|Suspicion")
	FVector GetLastKnownStimulusLocation() const { return LastKnownStimulusLocation; }

	/** Returns true if a stimulus location has been recorded. */
	UFUNCTION(BlueprintCallable, Category = "AI|Suspicion")
	bool HasStimulusLocation() const { return bHasStimulusLocation; }

	/** Set the stimulus location (used by sight to track player position). */
	void SetStimulusLocation(const FVector& Location)
	{
		LastKnownStimulusLocation = Location;
		bHasStimulusLocation = true;
	}

	/** Clear the stored stimulus location. */
	UFUNCTION(BlueprintCallable, Category = "AI|Suspicion")
	void ClearStimulusLocation();

	/** Get the type of the last stimulus that raised suspicion. */
	UFUNCTION(BlueprintCallable, Category = "AI|Suspicion")
	EStimulusType GetLastStimulusType() const { return LastStimulusType; }

	/** Set the last stimulus type (typically called internally by Process methods). */
	void SetLastStimulusType(EStimulusType Type) { LastStimulusType = Type; }

	/** Reset suspicion to zero and return to Patrol state. */
	UFUNCTION(BlueprintCallable, Category = "AI|Suspicion")
	void ResetSuspicion();

	// --- Delegate ---

	/** Broadcast when the alert level changes. */
	UPROPERTY(BlueprintAssignable, Category = "AI|Suspicion")
	FOnAlertLevelChanged OnAlertLevelChanged;

	// --- Tunable Properties ---

	/** Player visibility score below this threshold is invisible to the AI. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Suspicion|Tuning")
	float VisibilityThreshold = AIConstants::VisibilityThreshold;

	/** Base suspicion gain per second at full visibility. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Suspicion|Tuning")
	float BaseSuspicionRate = AIConstants::BaseSuspicionRate;

	/** Suspicion decay per second when no stimulus is present (~15s from full). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Suspicion|Tuning")
	float SuspicionDecayRate = AIConstants::SuspicionDecayRate;

	/** Suspicion value that triggers transition to Suspicious alert level. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Suspicion|Tuning")
	float SuspiciousThreshold = AIConstants::SuspiciousThreshold;

	/** Suspicion value that triggers transition to Alert alert level. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Suspicion|Tuning")
	float AlertThreshold = AIConstants::AlertThreshold;

	/** Fixed suspicion bump applied when a hearing stimulus is received. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Suspicion|Tuning")
	float HearingSuspicionBump = 0.25f;

private:
	/** Current suspicion value, clamped to [0.0, 1.0]. */
	UPROPERTY(VisibleAnywhere, Category = "AI|Suspicion|Debug")
	float CurrentSuspicion = 0.0f;

	/** Current alert level derived from suspicion thresholds. */
	UPROPERTY(VisibleAnywhere, Category = "AI|Suspicion|Debug")
	EAlertLevel CurrentAlertLevel = EAlertLevel::Patrol;

	/** Last known world location of a sight or hearing stimulus. */
	FVector LastKnownStimulusLocation = FVector::ZeroVector;

	/** Whether a stimulus location has been recorded since last clear. */
	bool bHasStimulusLocation = false;

	/** Type of the last stimulus that raised suspicion. Persists until full reset. */
	EStimulusType LastStimulusType = EStimulusType::None;

	/** Evaluate suspicion thresholds and broadcast if alert level changed. */
	void UpdateAlertLevel();
};
