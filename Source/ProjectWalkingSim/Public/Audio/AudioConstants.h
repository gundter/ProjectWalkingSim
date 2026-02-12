// Copyright Null Lantern.
#pragma once

/**
 * Audio tuning constants for The Juniper Tree.
 * Centralized defaults for audio systems added in Phase 6.
 * Individual components may override via UPROPERTY editable values.
 */
namespace AudioConstants
{
	// --- Monster Audio ---

	/** Default footstep interval at patrol speed (seconds). */
	constexpr float MonsterFootstepInterval_Patrol = 0.8f;

	/** Footstep interval during investigation (seconds). */
	constexpr float MonsterFootstepInterval_Investigate = 0.6f;

	/** Footstep interval during chase (seconds). */
	constexpr float MonsterFootstepInterval_Chase = 0.35f;

	/** Footstep interval during search (seconds). */
	constexpr float MonsterFootstepInterval_Search = 0.7f;

	/** Monster breathing crossfade duration when switching states (seconds). */
	constexpr float BreathingCrossfadeDuration = 1.5f;

	/** Chance (0-1) of a one-shot vocalization when entering a new behavior state. */
	constexpr float VocalizationChance = 0.6f;

	// --- Music Tension ---

	/** Default music crossfade duration (seconds). */
	constexpr float MusicCrossfadeDuration = 3.0f;

	/** Volume for the active music layer (0-1). */
	constexpr float MusicActiveVolume = 0.7f;

	// --- Ambient Audio ---

	/** Minimum interval between random one-shot ambient sounds (seconds). */
	constexpr float AmbientOneShotMinInterval = 5.0f;

	/** Maximum interval between random one-shot ambient sounds (seconds). */
	constexpr float AmbientOneShotMaxInterval = 20.0f;

	/** Predator silence fade-out duration (seconds). */
	constexpr float PredatorSilenceFadeOut = 2.0f;

	/** Predator silence fade-in (restore) duration (seconds). */
	constexpr float PredatorSilenceFadeIn = 3.0f;

	// --- Player Proximity ---

	/** Maximum distance (cm) at which heartbeat activates (~30m). */
	constexpr float HeartbeatMaxDistance = 3000.0f;

	/** Distance (cm) at which heartbeat is at max intensity (~5m). */
	constexpr float HeartbeatMinDistance = 500.0f;

	/** Update rate for heartbeat proximity check (Hz). */
	constexpr float HeartbeatUpdateRate = 4.0f;

	// --- Player Footstep Audio ---

	/** Base volume multiplier for player footstep audio playback. */
	constexpr float PlayerFootstepBaseVolume = 0.6f;

	// --- Flashlight ---

	/** Interval (seconds) for flashlight-to-Wendigo detection trace. */
	constexpr float FlashlightDetectionInterval = 0.5f;

	/** Half-angle (degrees) of the flashlight detection cone. */
	constexpr float FlashlightDetectionHalfAngle = 15.0f;

	/** Range (cm) of the flashlight detection trace (~15m, matches light attenuation). */
	constexpr float FlashlightDetectionRange = 1500.0f;

	/**
	 * Visibility score passed to ProcessSightStimulus when the flashlight beam
	 * hits the Wendigo. Lower values = slower suspicion accumulation.
	 *
	 * With BaseSuspicionRate=1.5 and VisibilityThreshold=0.05:
	 *   EffectiveVis = (0.3 - 0.05) / 0.95 = 0.263
	 *   Gain/call = 1.5 * 0.263 * 0.5 = ~0.20 per 0.5s
	 *   Time to Alert (0.8) = ~2.0s of sustained beam contact
	 *
	 * Tunable: increase for faster flashlight detection, decrease for slower.
	 */
	constexpr float FlashlightSuspicionScore = 0.3f;
}
