// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Chaos/ChaosEngineInterface.h" // EPhysicalSurface
#include "PlayerAudioComponent.generated.h"

class UAudioComponent;
class AWendigoCharacter;

/**
 * Player audio component for The Juniper Tree.
 *
 * Handles two player-side audio systems:
 *
 * 1. **Footstep playback**: Listens to FootstepComponent::OnFootstep and plays
 *    surface-type-aware footstep sounds as 2D audio (the player hears their own
 *    footsteps non-spatially).
 *
 * 2. **Heartbeat proximity**: A timer-based (4 Hz) proximity check against the
 *    nearest Wendigo that drives a MetaSound Intensity parameter on a looping
 *    heartbeat sound. Closer = faster/louder heartbeat. This is the player's
 *    non-directional intensity indicator of monster proximity (paired with
 *    MonsterAudioComponent's spatial breathing for directional awareness).
 *
 * No Tick -- footsteps are event-driven and heartbeat uses a timer.
 */
UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class PROJECTWALKINGSIM_API UPlayerAudioComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerAudioComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// --- Footstep Sound Properties ---

	/** Surface-specific footstep sounds. Key is the physical surface type. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Audio|Footsteps")
	TMap<TEnumAsByte<EPhysicalSurface>, TSoftObjectPtr<USoundBase>> FootstepSounds;

	/** Fallback footstep sound when surface type is not in the map. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Audio|Footsteps")
	TSoftObjectPtr<USoundBase> DefaultFootstepSound;

	/** Base volume multiplier for footstep playback. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Audio|Footsteps", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float FootstepBaseVolume = 0.6f;

	// --- Heartbeat Properties ---

	/** Looping heartbeat sound (MetaSound with Intensity parameter). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Audio|Heartbeat")
	TSoftObjectPtr<USoundBase> HeartbeatSound;

	/** Distance (cm) at which heartbeat starts (~30m). Beyond this, no heartbeat. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Audio|Heartbeat", meta = (ClampMin = "0.0"))
	float HeartbeatMaxDistance = 3000.0f;

	/** Distance (cm) at which heartbeat is max intensity (~5m). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Audio|Heartbeat", meta = (ClampMin = "0.0"))
	float HeartbeatMinDistance = 500.0f;

	/** How many times per second to check Wendigo proximity (Hz). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Audio|Heartbeat", meta = (ClampMin = "1.0", ClampMax = "30.0"))
	float HeartbeatUpdateRate = 4.0f;

	/** Name of the MetaSound float parameter controlling heartbeat intensity. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Audio|Heartbeat")
	FName HeartbeatIntensityParam = FName("Intensity");

private:
	/** Non-spatialized (2D) heartbeat audio component. */
	UPROPERTY()
	TObjectPtr<UAudioComponent> HeartbeatAudioComp;

	/** Timer handle for proximity-based heartbeat updates. */
	FTimerHandle HeartbeatTimerHandle;

	/** Cached reference to the Wendigo for distance checks (weak to handle respawn). */
	TWeakObjectPtr<AActor> CachedWendigo;

	// --- Internal Methods ---

	/** Delegate handler for FootstepComponent::OnFootstep. */
	UFUNCTION()
	void HandleFootstep(EPhysicalSurface SurfaceType, float Volume);

	/** Timer callback: checks distance to Wendigo and sets heartbeat intensity. */
	void UpdateHeartbeatProximity();

	/** Locates the nearest Wendigo in the world and caches its reference. */
	void FindWendigo();
};
