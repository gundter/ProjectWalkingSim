// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AmbientAudioManager.generated.h"

class UAudioComponent;
class USoundAttenuation;

/**
 * Manages the ambient soundscape for a level.
 * Plays a continuous looping ambient bed and randomized environmental one-shot sounds
 * on a timer. Supports "predator silence" -- fading all ambient to zero before the
 * Wendigo's appearance for maximum dread.
 *
 * Placed in the level as an actor. No Tick -- all timer-driven.
 */
UCLASS()
class PROJECTWALKINGSIM_API AAmbientAudioManager : public AActor
{
	GENERATED_BODY()

public:
	AAmbientAudioManager();

	// --- Public API ---

	/** Fade all ambient audio to silence (predator silence moment). */
	UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
	void EnterPredatorSilence(float FadeDuration = 2.0f);

	/** Restore ambient audio from silence. */
	UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
	void ExitPredatorSilence(float FadeDuration = 3.0f);

	/** Query whether the ambient is currently silenced. */
	UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
	bool IsSilenced() const { return bIsSilenced; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// --- Ambient Bed ---

	/** Looping ambient bed sound (always playing unless silenced). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Ambient|Bed")
	TSoftObjectPtr<USoundBase> AmbientBedSound;

	/** Base volume for the ambient bed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Ambient|Bed", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AmbientBedVolume = 0.5f;

	// --- Environmental One-Shots ---

	/** Pool of one-shot environmental sounds for randomized playback (creaks, drips, whispers). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Ambient|OneShots")
	TArray<TSoftObjectPtr<USoundBase>> EnvironmentalOneShotSounds;

	/** Min/max interval in seconds between random one-shot plays. X=min, Y=max. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Ambient|OneShots")
	FVector2D OneShotIntervalRange = FVector2D(5.0f, 20.0f);

	/** Volume variance for one-shots (+/- from base 0.5). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Ambient|OneShots", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float OneShotVolumeRange = 0.3f;

	/** Optional attenuation for 3D-positioned one-shots. If null, plays as 2D. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Ambient|OneShots")
	TObjectPtr<USoundAttenuation> OneShotAttenuation;

private:
	/** Audio component for the looping ambient bed. */
	UPROPERTY()
	TObjectPtr<UAudioComponent> AmbientBedComp;

	/** Timer handle for scheduling random one-shot playback. */
	FTimerHandle OneShotTimerHandle;

	/** Whether ambient is currently in predator silence mode. */
	bool bIsSilenced = false;

	/** Play a random one-shot from the pool. */
	void PlayRandomOneShot();

	/** Schedule the next random one-shot play after a random interval. */
	void ScheduleNextOneShot();
};
