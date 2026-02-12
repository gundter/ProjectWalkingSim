// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AI/MonsterAITypes.h"
#include "MonsterAudioComponent.generated.h"

class UAudioComponent;
class USoundAttenuation;

/**
 * Behavior-state-driven spatial audio component for the Wendigo.
 *
 * Manages three audio layers:
 *   1. Breathing -- looping 3D audio that crossfades when behavior state changes
 *   2. Footsteps -- timer-driven 3D positional sounds at the Wendigo's location
 *   3. Vocalizations -- one-shot growls/snarls on behavior state entry
 *
 * All audio is spatialized (3D positioned) so the player can locate the monster
 * by sound. Monster sounds do NOT report to UAISense_Hearing -- they exist
 * purely for the player's spatial awareness.
 *
 * No Tick overhead: fully event-driven (delegates) and timer-driven.
 */
UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class PROJECTWALKINGSIM_API UMonsterAudioComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMonsterAudioComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// --- Sound Assets (assigned in editor/Blueprint) ---

	/** Looping breathing sound per behavior state. Crossfades on state change. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster Audio|Breathing")
	TMap<EWendigoBehaviorState, TSoftObjectPtr<USoundBase>> BreathingSounds;

	/** Footstep sound for the Wendigo. Played at location on a timer. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster Audio|Footsteps")
	TSoftObjectPtr<USoundBase> FootstepSound;

	/** One-shot vocalizations (growl, sniff, snarl) per behavior state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster Audio|Vocalizations")
	TMap<EWendigoBehaviorState, TSoftObjectPtr<USoundBase>> VocalizationSounds;

	/** 3D spatial attenuation settings for all monster sounds. Assigned in editor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster Audio|Attenuation")
	TObjectPtr<USoundAttenuation> MonsterAttenuation;

	// --- Tuning ---

	/** Duration in seconds to crossfade between breathing sounds on state change. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster Audio|Tuning", meta = (ClampMin = "0.0"))
	float BreathingCrossfadeDuration = 1.5f;

	/** Chance (0-1) to play a vocalization when entering a new behavior state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster Audio|Tuning", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VocalizationChance = 0.6f;

private:
	/** Persistent looping audio component for breathing sounds. */
	UPROPERTY()
	TObjectPtr<UAudioComponent> BreathingAudioComp;

	/** Timer handle for footstep playback at intervals. */
	FTimerHandle FootstepTimerHandle;

	// --- Delegate handler ---

	/** Called when the owning WendigoCharacter changes behavior state. */
	UFUNCTION()
	void OnBehaviorStateChanged(EWendigoBehaviorState NewState);

	// --- Internal methods ---

	/** Crossfade breathing audio to match the given behavior state. */
	void TransitionBreathingSound(EWendigoBehaviorState NewState);

	/** Play a 3D positional footstep at the owner's location. */
	void PlayMonsterFootstep();

	/** Play a one-shot vocalization for the given state (subject to VocalizationChance). */
	void PlayVocalization(EWendigoBehaviorState State);

	/** Returns the footstep interval in seconds for the current behavior state. */
	float GetFootstepInterval() const;

	/** Restart the footstep timer with the interval appropriate for the current state. */
	void UpdateFootstepTimer();
};
