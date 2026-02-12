# Phase 6: Light and Audio - Research

**Researched:** 2026-02-12
**Domain:** UE5 Lighting (Lumen GI, SpotLight), Audio (MetaSounds, Spatial Audio, Dynamic Music), AI-Audio Integration
**Confidence:** HIGH

## Summary

Phase 6 builds the atmosphere infrastructure for a psychological horror game in UE 5.7.2 with C++. This phase delivers six C++ systems: a flashlight component, Lumen GI project configuration, spatial audio on the Wendigo, an ambient soundscape manager, monster audio cues tied to behavior states, and a dynamic music/tension system driven by AI alert level.

The existing codebase provides strong integration points: `EWendigoBehaviorState` and `FOnBehaviorStateChanged` on WendigoCharacter for driving monster audio states, `EAlertLevel` and `FOnAlertLevelChanged` on SuspicionComponent for driving music intensity, `FOnFootstep` on FootstepComponent for hooking audio playback to the player's existing footstep system, and `NoiseReportingComponent` as a pattern for how monster sounds can report to AI hearing perception. The VisibilityScoreComponent samples ambient light only (per CONTEXT decision, flashlight does NOT affect it).

The standard UE5 approach uses `UAudioComponent` for all runtime audio control, with `SetFloatParameter`/`SetBoolParameter` to drive MetaSound inputs from gameplay C++. MetaSounds are the modern replacement for SoundCues in UE5 and should be preferred for any procedural or parameter-driven audio. Sound assets (SoundWave, MetaSound Source) are created in-editor and referenced via `TSoftObjectPtr<USoundBase>` from C++ components. Sound Classes and Submixes provide the mixing hierarchy (Master > Music, SFX, Ambient, Monster).

**Primary recommendation:** Build six UActorComponents/subsystems in C++: FlashlightComponent, AmbientAudioManager (world subsystem or actor), MonsterAudioComponent, MusicTensionSystem (world subsystem), plus Lumen/VSM project configuration. All audio components use UAudioComponent internally and expose `TSoftObjectPtr<USoundBase>` properties for asset assignment in Blueprint/Editor. MetaSounds are preferred for anything parameter-driven (music intensity, ambient randomization), SoundWaves/SoundCues are acceptable for simple one-shot stingers.

## Standard Stack

### Core (Already in Project)
| Library/Module | Version | Purpose | Notes |
|----------------|---------|---------|-------|
| Engine | 5.7.2 | USpotLightComponent, UAudioComponent, UGameplayStatics | Already a dependency |
| AIModule | 5.7.2 | UAISense_Hearing::ReportNoiseEvent | Already a dependency for monster noise |

### New Dependencies Required
| Module | Purpose | Add To |
|--------|---------|--------|
| (none required) | All audio classes are in Engine module | N/A |

**Important:** `UAudioComponent`, `USoundBase`, `USoundAttenuation`, `USoundConcurrency`, `USpotLightComponent` are all in the `Engine` module, already listed in Build.cs. No new module dependencies are needed for Phase 6's C++ systems. MetaSound assets are created in-editor and played via `UAudioComponent::SetSound()` / `UAudioComponent::Play()` which are Engine module APIs.

**Optional (if needed later):**
| Module | Purpose | When |
|--------|---------|------|
| MetasoundEngine | Only if creating MetaSound nodes in C++ | Not needed for Phase 6 -- we play MetaSounds via UAudioComponent |
| AudioMixer | Only if using Quartz clock for beat-synced music | Not needed for Phase 6's crossfade approach |

### Audio Asset Types (Editor-Created, C++ Referenced)
| Asset Type | UE Class | Usage |
|------------|----------|-------|
| Sound Wave | USoundWave | Raw audio files (.wav imports) |
| MetaSound Source | UMetaSoundSource (extends USoundBase) | Procedural/parameterized audio (ambient, music layers) |
| Sound Cue | USoundCue | Simple randomized one-shots (stingers, footstep variations) |
| Sound Attenuation | USoundAttenuation | 3D falloff, occlusion, spatialization settings |
| Sound Concurrency | USoundConcurrency | Limits simultaneous sounds per category |
| Sound Class | USoundClass | Mixing hierarchy (Master > Music, SFX, Ambient, Monster) |
| Sound Mix | USoundMix | Volume overrides per Sound Class |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| MetaSounds for music | SoundCues with Crossfade by Param | SoundCues are legacy; MetaSounds have better performance and parameter control |
| Built-in spatialization | Steam Audio / Resonance Audio plugin | Third-party plugins add HRTF binaural; overkill for demo scope |
| C++ audio subsystem | Wwise middleware | Industry standard but massive integration overhead; not justified for demo |
| UAudioComponent layers | Quartz beat-synced music | Quartz is powerful but complex; simple crossfade is sufficient for demo |

## Architecture Patterns

### Recommended Project Structure
```
Source/ProjectWalkingSim/
  Public/
    Audio/
      FlashlightComponent.h          # USpotLightComponent wrapper on player
      AmbientAudioManager.h          # Manages ambient sound layers
      MonsterAudioComponent.h        # Per-state audio on WendigoCharacter
      MusicTensionSystem.h           # Dynamic music driven by AI state
      AudioConstants.h               # Audio tuning constants namespace
  Private/
    Audio/
      FlashlightComponent.cpp
      AmbientAudioManager.cpp
      MonsterAudioComponent.cpp
      MusicTensionSystem.cpp
```

### Pattern 1: FlashlightComponent (UActorComponent owning a USpotLightComponent)

**What:** A component attached to SereneCharacter that creates and manages a USpotLightComponent attached to the camera. Always-on for the demo. Narrow beam, cool color temperature, deep shadows.

**When to use:** Single-responsibility component pattern matching existing codebase (StaminaComponent, HeadBobComponent, etc.).

**Key properties:**
```cpp
// FlashlightComponent.h
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UFlashlightComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UFlashlightComponent();

protected:
    virtual void BeginPlay() override;

    // --- Light Configuration ---

    /** Inner cone angle in degrees. Narrow beam for horror. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight|Beam")
    float InnerConeAngle = 14.0f;

    /** Outer cone angle in degrees. Penumbra falloff region. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight|Beam")
    float OuterConeAngle = 28.0f;

    /** Light intensity in lumens. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight|Beam")
    float Intensity = 8000.0f;

    /** Attenuation radius in cm (~15m range). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight|Beam")
    float AttenuationRadius = 1500.0f;

    /** Cool bluish-white LED color. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight|Beam")
    FLinearColor LightColor = FLinearColor(0.85f, 0.92f, 1.0f);

    /** Volumetric fog scattering intensity. Keep low to avoid ghosting artifacts. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight|Beam")
    float VolumetricScatteringIntensity = 1.0f;

private:
    /** The actual spot light -- created at runtime, attached to camera. */
    UPROPERTY()
    TObjectPtr<USpotLightComponent> SpotLight;
};
```

**Critical implementation detail:** The `USpotLightComponent` must be created at runtime in `BeginPlay()` using `NewObject<USpotLightComponent>()` and registered/attached, NOT via `CreateDefaultSubobject` (which only works in constructors for components owned by the actor, not by another component). The component attaches to the player's `FirstPersonCamera`.

```cpp
// FlashlightComponent.cpp - BeginPlay pattern
void UFlashlightComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    UCameraComponent* Camera = Owner->FindComponentByClass<UCameraComponent>();
    if (!Camera) return;

    SpotLight = NewObject<USpotLightComponent>(Owner, TEXT("FlashlightSpotLight"));
    SpotLight->SetupAttachment(Camera);
    SpotLight->SetInnerConeAngle(InnerConeAngle);
    SpotLight->SetOuterConeAngle(OuterConeAngle);
    SpotLight->SetIntensity(Intensity);
    SpotLight->SetAttenuationRadius(AttenuationRadius);
    SpotLight->SetLightColor(LightColor);
    SpotLight->SetVolumetricScatteringIntensity(VolumetricScatteringIntensity);
    SpotLight->SetCastShadows(true);
    SpotLight->SetMobility(EComponentMobility::Movable);
    SpotLight->RegisterComponent();
}
```

### Pattern 2: MonsterAudioComponent (Behavior-State-Driven Audio)

**What:** A component on AWendigoCharacter that listens to `OnBehaviorStateChanged` and crossfades between audio layers per state. Each state has a different audio profile (breathing type, footstep intensity, vocalization).

**When to use:** Placed on WendigoCharacter, binds to the existing `FOnBehaviorStateChanged` delegate.

```cpp
// MonsterAudioComponent.h
UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class UMonsterAudioComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UMonsterAudioComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;

    // --- Sound Assets (assigned in editor/Blueprint) ---

    /** Breathing sounds per behavior state. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster Audio|Breathing")
    TMap<EWendigoBehaviorState, TSoftObjectPtr<USoundBase>> BreathingSounds;

    /** Footstep sound for the Wendigo. Played on timer, scaled by movement speed. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster Audio|Footsteps")
    TSoftObjectPtr<USoundBase> FootstepSound;

    /** Alert vocalizations (growl, sniff, snarl) per state. One-shots. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster Audio|Vocalizations")
    TMap<EWendigoBehaviorState, TSoftObjectPtr<USoundBase>> VocalizationSounds;

    /** Attenuation settings for 3D spatial audio. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster Audio|Attenuation")
    TObjectPtr<USoundAttenuation> MonsterAttenuation;

private:
    UPROPERTY()
    TObjectPtr<UAudioComponent> BreathingAudioComp;

    UPROPERTY()
    TObjectPtr<UAudioComponent> FootstepAudioComp;

    UFUNCTION()
    void OnBehaviorStateChanged(EWendigoBehaviorState NewState);

    void TransitionBreathingSound(EWendigoBehaviorState NewState);

    // Timer-based monster footsteps
    FTimerHandle FootstepTimerHandle;
    void PlayMonsterFootstep();
    float GetFootstepInterval() const; // varies by behavior state (speed)
};
```

### Pattern 3: MusicTensionSystem (Alert-Level-Driven Music)

**What:** A subsystem or actor that owns multiple UAudioComponents for music layers, crossfading between intensity tiers driven by the Wendigo's AlertLevel. Uses `UAudioComponent::FadeIn()` / `UAudioComponent::FadeOut()` for smooth transitions.

**Architecture:**
- 3 music layers: Calm (Patrol), Tense (Suspicious), Intense (Chase/Alert)
- Each layer is a looping UAudioComponent
- AlertLevel changes trigger crossfade: fade out current, fade in target
- Stingers are one-shot UAudioComponents triggered by specific events

```cpp
// MusicTensionSystem.h
UENUM(BlueprintType)
enum class EMusicIntensity : uint8
{
    Calm,      // Patrol -- ambient drone, sparse
    Tense,     // Suspicious -- building tension
    Intense,   // Alert (Chase/Search/GrabAttack) -- full intensity
    Silent     // Special: predator silence moment
};

UCLASS()
class UMusicTensionSystem : public UActorComponent
{
    GENERATED_BODY()
public:
    UMusicTensionSystem();

    /** Transition to a new music intensity with crossfade. */
    UFUNCTION(BlueprintCallable, Category = "Music")
    void SetMusicIntensity(EMusicIntensity NewIntensity);

    /** Play a one-shot stinger sound (non-positional). */
    UFUNCTION(BlueprintCallable, Category = "Music")
    void PlayStinger(USoundBase* StingerSound);

protected:
    virtual void BeginPlay() override;

    /** Music assets per intensity tier (assigned in editor). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music|Tracks")
    TSoftObjectPtr<USoundBase> CalmMusic;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music|Tracks")
    TSoftObjectPtr<USoundBase> TenseMusic;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music|Tracks")
    TSoftObjectPtr<USoundBase> IntenseMusic;

    /** Stinger sounds for specific events. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music|Stingers")
    TSoftObjectPtr<USoundBase> SpottedStinger;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music|Stingers")
    TSoftObjectPtr<USoundBase> EscapedStinger;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music|Stingers")
    TSoftObjectPtr<USoundBase> GrabAttackStinger;

    /** Crossfade duration in seconds. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music|Transition")
    float CrossfadeDuration = 3.0f;

private:
    UPROPERTY()
    TObjectPtr<UAudioComponent> MusicLayers[3]; // Calm, Tense, Intense

    UPROPERTY()
    TObjectPtr<UAudioComponent> StingerComp;

    EMusicIntensity CurrentIntensity = EMusicIntensity::Calm;

    UFUNCTION()
    void OnAlertLevelChanged(EAlertLevel NewLevel);

    EMusicIntensity AlertLevelToIntensity(EAlertLevel Level) const;
    UAudioComponent* GetLayerComponent(EMusicIntensity Intensity) const;
};
```

### Pattern 4: AmbientAudioManager (Layered Ambient Soundscape)

**What:** An actor placed in the level that manages ambient sound layers. Supports indoor/outdoor ambient sets via Audio Volumes and Ambient Zones. Spawns randomized one-shot environmental sounds (creaks, drips, whispers) on timers.

**Architecture:**
- Base ambient bed: looping UAudioComponent (always playing)
- Randomized one-shots: timer-triggered at random intervals from a sound pool
- Indoor/outdoor: Use UE5's built-in Ambient Zone system on Audio Volumes (editor config, not C++)
- "Predator silence" support: method to fade all ambient to zero

```cpp
// AmbientAudioManager.h
UCLASS()
class AAmbientAudioManager : public AActor
{
    GENERATED_BODY()
public:
    AAmbientAudioManager();

    /** Fade all ambient audio to silence (predator silence moment). */
    UFUNCTION(BlueprintCallable, Category = "Ambient")
    void EnterPredatorSilence(float FadeDuration = 2.0f);

    /** Restore ambient audio from silence. */
    UFUNCTION(BlueprintCallable, Category = "Ambient")
    void ExitPredatorSilence(float FadeDuration = 3.0f);

protected:
    virtual void BeginPlay() override;

    /** Ambient bed sound (looping). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ambient|Bed")
    TSoftObjectPtr<USoundBase> AmbientBedSound;

    /** Pool of one-shot environmental sounds for random playback. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ambient|OneShots")
    TArray<TSoftObjectPtr<USoundBase>> EnvironmentalOneShotSounds;

    /** Min/max interval between random one-shots (seconds). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ambient|OneShots")
    FVector2D OneShotIntervalRange = FVector2D(5.0f, 20.0f);

private:
    UPROPERTY()
    TObjectPtr<UAudioComponent> AmbientBedComp;

    FTimerHandle OneShotTimerHandle;
    void PlayRandomOneShot();
    void ScheduleNextOneShot();
};
```

### Pattern 5: Player Heartbeat Proximity System

**What:** A non-positional UAudioComponent on the player that increases heartbeat intensity based on distance to the Wendigo. Uses `SetFloatParameter` to drive a MetaSound "Intensity" input.

```cpp
// On SereneCharacter or as a separate component
// Heartbeat audio component (2D, non-spatialized)
// Tick checks distance to Wendigo, maps to 0-1 intensity
// UAudioComponent->SetFloatParameter("Intensity", ProximityValue);
```

### Anti-Patterns to Avoid
- **Playing sounds directly from AI State Tree tasks:** State Tree tasks should call component methods, not manage UAudioComponents directly. Audio logic belongs in MonsterAudioComponent.
- **Tick-based audio parameter updates every frame:** Use timer-based updates (4-8 Hz) for audio parameter changes. Audio perception of parameter changes below ~100ms is negligible.
- **Creating UAudioComponents in constructors for dynamically-assigned sounds:** Use `NewObject` in BeginPlay or lazy-create on first play. Constructor-created components with null sounds log warnings.
- **Hard-referencing sound assets in C++ headers:** Use `TSoftObjectPtr<USoundBase>` to avoid loading all audio into memory at startup. Load on demand or in BeginPlay.
- **Putting the music system on a specific actor:** Music persists across level changes. Use a component on the GameInstance, PlayerController, or a singleton manager actor.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| 3D audio falloff curves | Custom distance-based volume math | USoundAttenuation asset | 5 built-in falloff models (Linear, Log, Inverse, LogReverse, NaturalSound), plus custom curves. Handles occlusion traces for free. |
| Sound through walls | Raycasting + manual volume reduction | USoundAttenuation occlusion settings | Built-in async trace with LPF + volume attenuation. Extremely cheap. Set `bEnableOcclusion=true`. |
| Indoor/outdoor ambient transitions | Custom volume blend on overlap | Audio Volumes + Ambient Zone Settings | Built-in system with configurable fade times, LPF, and volume multipliers. Editor-only configuration. |
| Limiting simultaneous sounds | Manual tracking of active sound count | USoundConcurrency asset | Resolution rules (StopOldest, StopQuietest, PreventNew, etc.), voice stealing, volume ducking. |
| Audio mixing hierarchy | Manual volume multipliers per component | Sound Classes + Sound Mixes | Hierarchical volume/pitch/LPF control. Master > Music/SFX/Ambient/Monster. |
| Smooth music transitions | Manual timer-based volume interpolation | UAudioComponent::FadeIn/FadeOut | Built-in fade with configurable duration, target volume, and curve type (EAudioFaderCurve). |
| Sound spatialization | Custom panning math | Attenuation Spatialization settings | Built-in panner or binaural plugin support. Non-spatialized radius for smooth transitions. |

**Key insight:** UE5's audio engine is extremely full-featured. Almost every audio behavior needed for a horror game has a built-in solution via asset configuration. The C++ layer should create/manage UAudioComponents, set parameters, and react to gameplay events -- NOT implement audio DSP, mixing, or spatialization logic.

## Common Pitfalls

### Pitfall 1: Volumetric Fog Ghosting with Flashlight
**What goes wrong:** Moving a spotlight rapidly (as with a head-tracked flashlight) causes visible light trails/ghosting in volumetric fog due to temporal reprojection.
**Why it happens:** Volumetric fog uses temporal jittering across frames to reduce aliasing. Fast-moving lights leave stale data in previous frame positions.
**How to avoid:** Keep `VolumetricScatteringIntensity` moderate (0.5-1.0). If ghosting is severe, reduce it to 0. Alternatively, reduce `r.VolumetricFog.GridSizeZ` for faster updates at higher GPU cost. The temporal filter is a fundamental design choice -- this cannot be fully eliminated.
**Warning signs:** Visible light streaks trailing behind the flashlight beam when the player looks around quickly.

### Pitfall 2: Dynamic Spotlight Performance with Lumen
**What goes wrong:** A movable spotlight with shadow casting + Lumen GI bounce can cost 2-5ms GPU per frame, especially in larger scenes.
**Why it happens:** Movable lights must update the Lumen Surface Cache every frame. Shadow-casting spot lights with volumetric fog are ~3x more expensive than non-shadow-casting.
**How to avoid:**
- Set `CastShadows=true` but tune `Shadow Resolution Scale` lower for the flashlight
- Keep the flashlight `AttenuationRadius` reasonable (~1500cm / 15m)
- Consider disabling `Cast Volumetric Shadow` on the spotlight (volumetric shadows are the expensive part -- 3x cost increase)
- Use `r.Lumen.ScreenProbeGather.ScreenSpaceTracingOcclusionConeHalfAngle` to tune
- Profile with `stat gpu` and `stat lumen`
**Warning signs:** GPU frame time > 10ms on target hardware. Use `stat gpu` to identify Lumen/Shadow/VolumetricFog breakdown.

### Pitfall 3: USpotLightComponent Creation in Wrong Context
**What goes wrong:** Attempting to create a USpotLightComponent via `CreateDefaultSubobject` inside another component's constructor causes "Using incorrect object initializer" error.
**Why it happens:** `CreateDefaultSubobject` can only be called during an Actor's constructor to create components owned by that actor. A component creating a sub-component must use `NewObject` + `RegisterComponent` in BeginPlay instead.
**How to avoid:** Create the USpotLightComponent in `BeginPlay()` using `NewObject<USpotLightComponent>(GetOwner(), TEXT("Name"))`, then call `SetupAttachment`, configure properties, and call `RegisterComponent()`.
**Warning signs:** Compile error mentioning "incorrect object initializer" or crash during CDO construction.

### Pitfall 4: Sound Asset Null References at Play Time
**What goes wrong:** `UAudioComponent::Play()` with no sound set logs warnings and plays nothing. `SetSound(nullptr)` also warns.
**Why it happens:** TSoftObjectPtr assets not loaded yet, or editor properties not assigned.
**How to avoid:** Always null-check before Play: `if (Sound.IsValid()) { AudioComp->SetSound(Sound.Get()); AudioComp->Play(); }`. For TSoftObjectPtr, call `LoadSynchronous()` in BeginPlay or use async loading.
**Warning signs:** "No sound set" warnings in output log during gameplay.

### Pitfall 5: Music System Doesn't Survive Level Transitions
**What goes wrong:** Music cuts out abruptly when loading a new level because the manager actor is destroyed.
**Why it happens:** Actors in a level are destroyed on level unload. The music system must persist.
**How to avoid:** For the demo (single level), this is not an issue. If needed later, store the music system on the GameInstance or use a persistent level. For now, placing the MusicTensionSystem as a component on a level actor is fine.
**Warning signs:** Music restarts from the beginning on level load.

### Pitfall 6: Monster Audio Competing with Music
**What goes wrong:** Monster growls, breathing, and footsteps become inaudible under intense chase music.
**Why it happens:** No mixing hierarchy to duck music when monster proximity audio is important.
**How to avoid:** Set up Sound Classes: Monster sounds at higher priority than Music. Use Sound Class passive mix modifiers to duck music volume when monster sounds play, or manually reduce music volume during Alert state. Sound Concurrency can also help limit total sound count.
**Warning signs:** Player can't hear directional monster audio cues during chase sequences.

### Pitfall 7: AI Hearing Perceives Monster's Own Sounds
**What goes wrong:** The Wendigo's hearing perception picks up its own footstep/breathing sounds, causing self-suspicion.
**Why it happens:** `UAISense_Hearing::ReportNoiseEvent` broadcasts to all listeners in range, including the source.
**How to avoid:** When reporting monster sounds to AI hearing (if at all -- monster sounds should NOT be reported), use the NoiseInstigator parameter set to the Wendigo itself. The AI perception system's affiliation filter then ignores self-generated noise. However, the simpler approach: don't report monster audio to the AI hearing system at all. Monster sounds are for the PLAYER to hear. Only player sprint noise reports to AI hearing.
**Warning signs:** Wendigo becomes suspicious of its own footsteps, enters investigation loop.

## Code Examples

### Creating a UAudioComponent at Runtime (Attached, 3D Spatial)
```cpp
// Source: UE5 Engine API - UAudioComponent
// Create a 3D spatial audio component attached to an actor
UAudioComponent* AudioComp = NewObject<UAudioComponent>(this);
AudioComp->SetupAttachment(GetRootComponent());
AudioComp->bAutoActivate = false;
AudioComp->bAutoDestroy = false;  // Managed lifetime
AudioComp->AttenuationSettings = MonsterAttenuation;  // USoundAttenuation* asset
AudioComp->RegisterComponent();

// Later, when playing:
if (USoundBase* Sound = BreathingSounds.FindRef(EWendigoBehaviorState::Patrol).LoadSynchronous())
{
    AudioComp->SetSound(Sound);
    AudioComp->Play();
}
```

### Crossfading Between Music Layers
```cpp
// Source: UAudioComponent API - FadeIn/FadeOut
void UMusicTensionSystem::SetMusicIntensity(EMusicIntensity NewIntensity)
{
    if (NewIntensity == CurrentIntensity) return;

    // Fade out current layer
    if (UAudioComponent* CurrentComp = GetLayerComponent(CurrentIntensity))
    {
        CurrentComp->FadeOut(CrossfadeDuration, 0.0f, EAudioFaderCurve::SCurve);
    }

    // Fade in new layer
    if (UAudioComponent* NewComp = GetLayerComponent(NewIntensity))
    {
        if (!NewComp->IsPlaying())
        {
            NewComp->Play();
        }
        NewComp->FadeIn(CrossfadeDuration, 1.0f, 0.0f, EAudioFaderCurve::SCurve);
    }

    CurrentIntensity = NewIntensity;
}
```

### Setting MetaSound Parameters from C++
```cpp
// Source: UAudioComponent API - SetFloatParameter
// Drive a MetaSound "Intensity" input from gameplay
void UHeartbeatComponent::UpdateHeartbeat(float ProximityNormalized)
{
    // ProximityNormalized: 0.0 = far away, 1.0 = very close
    if (HeartbeatAudioComp && HeartbeatAudioComp->IsPlaying())
    {
        HeartbeatAudioComp->SetFloatParameter(FName("Intensity"), ProximityNormalized);
    }
}
```

### Binding to Existing Delegates for Audio Triggers
```cpp
// Source: Existing codebase pattern (WendigoAIController::BindToPlayerDelegates)
void UMonsterAudioComponent::BeginPlay()
{
    Super::BeginPlay();

    // Bind to WendigoCharacter's existing behavior state delegate
    if (AWendigoCharacter* Wendigo = Cast<AWendigoCharacter>(GetOwner()))
    {
        Wendigo->OnBehaviorStateChanged.AddDynamic(
            this, &UMonsterAudioComponent::OnBehaviorStateChanged);
    }

    // Create audio components
    BreathingAudioComp = NewObject<UAudioComponent>(GetOwner());
    BreathingAudioComp->SetupAttachment(GetOwner()->GetRootComponent());
    BreathingAudioComp->bAutoActivate = false;
    BreathingAudioComp->bAutoDestroy = false;
    if (MonsterAttenuation)
    {
        BreathingAudioComp->AttenuationSettings = MonsterAttenuation;
    }
    BreathingAudioComp->RegisterComponent();
}
```

### Reporting Monster Footsteps as Player-Audible 3D Sound (NOT to AI)
```cpp
// Monster footsteps play spatialized audio for the player to hear
// but do NOT report to UAISense_Hearing (to avoid self-detection)
void UMonsterAudioComponent::PlayMonsterFootstep()
{
    USoundBase* Sound = FootstepSound.LoadSynchronous();
    if (!Sound) return;

    // Play 3D positional sound at monster's feet
    UGameplayStatics::PlaySoundAtLocation(
        this,
        Sound,
        GetOwner()->GetActorLocation(),
        FRotator::ZeroRotator,
        1.0f,   // VolumeMultiplier
        1.0f,   // PitchMultiplier
        0.0f,   // StartTime
        MonsterAttenuation,
        nullptr, // ConcurrencySettings
        nullptr  // OwningActor - null means no AI noise report
    );
}
```

### Wiring Music System to AlertLevel
```cpp
// In MusicTensionSystem::BeginPlay, find the Wendigo and bind
void UMusicTensionSystem::BeginPlay()
{
    Super::BeginPlay();

    // Find Wendigo in world and bind to its SuspicionComponent
    // (or receive delegate from game mode / event system)
    TArray<AActor*> Wendigos;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWendigoCharacter::StaticClass(), Wendigos);
    if (Wendigos.Num() > 0)
    {
        AWendigoCharacter* Wendigo = Cast<AWendigoCharacter>(Wendigos[0]);
        if (Wendigo && Wendigo->GetSuspicionComponent())
        {
            Wendigo->GetSuspicionComponent()->OnAlertLevelChanged.AddDynamic(
                this, &UMusicTensionSystem::OnAlertLevelChanged);
        }
    }

    // Create and start all music layer components (all playing, volumes control which is heard)
    // ... create UAudioComponents for Calm, Tense, Intense layers ...
}

void UMusicTensionSystem::OnAlertLevelChanged(EAlertLevel NewLevel)
{
    SetMusicIntensity(AlertLevelToIntensity(NewLevel));
}

EMusicIntensity UMusicTensionSystem::AlertLevelToIntensity(EAlertLevel Level) const
{
    switch (Level)
    {
        case EAlertLevel::Patrol:     return EMusicIntensity::Calm;
        case EAlertLevel::Suspicious: return EMusicIntensity::Tense;
        case EAlertLevel::Alert:      return EMusicIntensity::Intense;
        default:                      return EMusicIntensity::Calm;
    }
}
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| SoundCues for all audio | MetaSounds for procedural/parameterized audio | UE 5.0+ | MetaSounds compile to optimized C++ DSP; no interpreted bytecode overhead. Better for parameter-driven audio. |
| Legacy audio engine | New Audio Engine (AudioMixer) | UE 4.24+ | Required for spatialization plugins, submix effects. Should be enabled by default in 5.7. |
| Lightmass baked GI | Lumen real-time GI | UE 5.0+ | Dynamic spotlights (flashlights) now contribute to GI bounce. No bake step. |
| Shadow Maps (traditional) | Virtual Shadow Maps | UE 5.0+ | Per-pixel shadow resolution, better quality for dynamic lights at scale. |
| PawnSensing + MakeNoise | AIPerception + ReportNoiseEvent | UE 4.20+ | Already using the correct modern system in the existing codebase. |

**Deprecated/outdated:**
- **SoundCues for complex audio:** Still functional but MetaSounds are the recommended path for anything non-trivial. SoundCues are fine for simple randomized one-shots.
- **Legacy audio engine:** Must use AudioMixer backend (default in UE5). Verify in Project Settings > Platforms > Audio.
- **Static lights with Lumen:** Static mobility lights are completely ignored when Lumen is enabled. All lights must be Movable or Stationary.

## Lumen GI Configuration (Project Settings)

These settings are configured in Project Settings and PostProcessVolume, NOT in C++ code:

### Project Settings (Engine > Rendering)
| Setting | Value | Notes |
|---------|-------|-------|
| Dynamic Global Illumination Method | Lumen | Enables Lumen GI |
| Reflection Method | Lumen | Lumen reflections |
| Generate Mesh Distance Fields | true | Required for software ray tracing |
| Shadow Map Method | Virtual Shadow Maps | Per-pixel shadow quality |

### PostProcessVolume Settings (Unbound, in level)
| Setting | Recommended Value | Notes |
|---------|-------------------|-------|
| Lumen Scene Lighting Quality | 1.0 (Medium) | Balance quality vs GPU cost |
| Lumen Scene Detail | 1.0 | Default; increase for small object GI |
| Final Gather Quality | 1.0 | Reduce noise; increase = more GPU |
| Lumen Scene Lighting Update Speed | 1.0 | Higher = faster propagation, more GPU |
| Max Trace Distance | 20000 | Prevents light leaks |
| Screen Traces | true | Better quality for screen-space GI |

### Key Console Variables for Performance Tuning
| CVar | Purpose |
|------|---------|
| `r.Lumen.Reflections.MaxBounces` | Reflection bounce count (default 2, max 64) |
| `r.VolumetricFog.GridSizeZ` | Volumetric fog Z-resolution |
| `r.Lumen.Visualize.CardPlacement 1` | Debug: visualize surface cache coverage |
| `r.Lumen.ScreenProbeGather` | Control screen-space probe settings |
| `stat gpu` | Profile GPU frame time breakdown |
| `stat lumen` | Lumen-specific performance stats |

### Exponential Height Fog (for Volumetric Fog)
| Setting | Value | Notes |
|---------|-------|-------|
| Volumetric Fog | true | Required for spotlight visible beams |
| Fog Density | Very low (0.001-0.01) | Horror: subtle fog, not pea-soup |
| Albedo | Dark gray | Light absorption for moody atmosphere |

## Integration with Existing Systems

### Flashlight -> VisibilityScoreComponent
**Decision (from CONTEXT.md): Flashlight does NOT affect visibility score.** VisibilityScoreComponent uses a SceneCapture with `SetPostProcessing(false)`, sampling ambient light only. The flashlight is a Movable SpotLight that will appear in the SceneCapture's render. However, since the SceneCapture is positioned at the player and captures a tiny 8x8 downsampled view of the surroundings, the flashlight's directional beam pointing AWAY from the player should have minimal effect on the player's own visibility reading. If testing reveals the flashlight does raise VisibilityScore, options include:
1. Setting the SpotLight to a custom rendering channel excluded from the SceneCapture
2. Disabling the SpotLight during capture frames (via the capture timer callback)
3. Adjusting the SceneCapture to face upward/downward rather than forward

### Monster Audio -> AI Hearing System
**Monster sounds are for the PLAYER, not for AI.** The existing `NoiseReportingComponent` pattern on SereneCharacter reports player sprint noise to `UAISense_Hearing`. Monster sounds should NOT be reported to AI hearing (the Wendigo shouldn't hear itself). Monster audio is purely spatial audio for the player's benefit -- 3D positioned so the player can locate the monster by sound. Use `UGameplayStatics::PlaySoundAtLocation` or attached `UAudioComponent` without `ReportNoiseEvent`.

**However, the flashlight attracting the Wendigo (per CONTEXT decision)** needs a mechanism: When the flashlight beam hits near the Wendigo, report a sight stimulus or custom perception event. This could be implemented as a line trace from the flashlight checking if the beam cone intersects the Wendigo, then calling a method on the WendigoAIController to process "flashlight detected" as a visual stimulus. This is a Phase 6 integration point.

### Music System -> SuspicionComponent
**Wiring:** `SuspicionComponent::OnAlertLevelChanged` delegate drives `MusicTensionSystem::SetMusicIntensity()`. The mapping is:
- `EAlertLevel::Patrol` -> `EMusicIntensity::Calm`
- `EAlertLevel::Suspicious` -> `EMusicIntensity::Tense`
- `EAlertLevel::Alert` -> `EMusicIntensity::Intense`

### Footstep Audio -> Existing FootstepComponent
**Wiring:** `FootstepComponent::OnFootstep` delegate already broadcasts `(EPhysicalSurface, float Volume)`. Phase 6 adds a listener that plays actual sound assets based on surface type. This could be a small component on SereneCharacter or directly handled in a callback. The existing `NoiseReportingComponent` already listens to `OnFootstep` for AI noise -- the audio playback listener is separate.

### Stinger Events -> Behavior State Changes
**Wiring:** Stingers fire on specific transitions:
- Monster spots player: `EAlertLevel::Suspicious -> Alert` (first time only per encounter)
- Player escapes chase: `EWendigoBehaviorState::Chasing -> Searching`
- Grab attack: `EWendigoBehaviorState::GrabAttack` entered

## Open Questions

1. **Flashlight beam attracting Wendigo - exact mechanism**
   - What we know: CONTEXT says "shining at or near the Wendigo draws its attention"
   - What's unclear: Should this be a cone trace from flashlight? A special perception sense? A periodic check?
   - Recommendation: Implement as a periodic trace (every 0.5s) from flashlight checking if beam cone overlaps with Wendigo. If so, report as a visual stimulus to SuspicionComponent. Keep it simple for demo.

2. **Player heartbeat proximity implementation detail**
   - What we know: CONTEXT specifies dual proximity cue (heartbeat + spatial breathing)
   - What's unclear: How to calculate proximity -- just distance? Or should it consider line of sight?
   - Recommendation: Pure distance-based, sampled on a timer (4Hz). Distance to nearest Wendigo, normalized to 0-1 over a configurable range (e.g., 0-30m). MetaSound with Intensity parameter for heartbeat rate/volume.

3. **Predator silence trigger mechanism**
   - What we know: All ambient drops out before the Wendigo's first introduction
   - What's unclear: This is a scripted moment -- how does it trigger? Level sequence? Blueprint event?
   - Recommendation: Expose `EnterPredatorSilence()` / `ExitPredatorSilence()` on the AmbientAudioManager. Trigger mechanism is Phase 8 (Demo Polish) concern -- Phase 6 just builds the API.

4. **Sound asset pipeline**
   - What we know: Phase 6 is infrastructure, not content. Phase 8 creates actual sound assets.
   - What's unclear: Should Phase 6 include placeholder test sounds?
   - Recommendation: Create minimal placeholder sounds (beep/tone) for verification testing only. Real assets are Phase 8.

## Sources

### Primary (HIGH confidence)
- UE 5.7 Documentation: [Lumen Global Illumination](https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-global-illumination-and-reflections-in-unreal-engine) - GI settings, light support, performance
- UE 5.7 Documentation: [Lumen Technical Details](https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-technical-details-in-unreal-engine) - Surface cache, console variables
- UE 5.7 Documentation: [Sound Attenuation](https://dev.epicgames.com/documentation/en-us/unreal-engine/sound-attenuation-in-unreal-engine) - Occlusion, spatialization, falloff models
- UE 5.7 Documentation: [Sound Concurrency](https://dev.epicgames.com/documentation/en-us/unreal-engine/sound-concurrency-reference-guide) - Limiting simultaneous sounds
- UE 5.7 Documentation: [Sound Classes](https://dev.epicgames.com/documentation/en-us/unreal-engine/sound-classes-in-unreal-engine) - Mixing hierarchy
- UE 5.7 Documentation: [Submixes](https://dev.epicgames.com/documentation/en-us/unreal-engine/overview-of-submixes-in-unreal-engine) - DSP graph, effects, routing
- UE 5.7 Documentation: [Volumetric Fog](https://dev.epicgames.com/documentation/en-us/unreal-engine/volumetric-fog-in-unreal-engine) - Fog setup, ghosting issue, performance
- UE 5.7 Documentation: [Spot Lights](https://dev.epicgames.com/documentation/en-us/unreal-engine/spot-lights-in-unreal-engine) - Properties, shadow settings
- UE 5.7 Documentation: [Ambient Zones](https://dev.epicgames.com/documentation/en-us/unreal-engine/ambient-zones-in-unreal-engine) - Indoor/outdoor transitions
- UE 5.7 Documentation: [MetaSounds](https://dev.epicgames.com/documentation/en-us/unreal-engine/metasounds-the-next-generation-sound-sources-in-unreal-engine) - Next-gen audio, parameter interface
- UE 5.7 API: [UAudioComponent](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/UAudioComponent) - Full method list
- Existing codebase: WendigoCharacter.h, SuspicionComponent.h, FootstepComponent.h, NoiseReportingComponent.h, WendigoAIController.cpp

### Secondary (MEDIUM confidence)
- Lyra Music System Analysis: [disasterpeace.com/blog/epic-games.lyra.html](https://disasterpeace.com/blog/epic-games.lyra.html) - Epic's official sample project music architecture
- Epic Forums: USpotLightComponent initialization pattern - [forums.unrealengine.com](https://forums.unrealengine.com/t/using-incorrect-object-initializer-while-initializing-uspotlightcomponent/687395)

### Tertiary (LOW confidence)
- Various WebSearch results for horror audio design patterns - general game design wisdom, not UE5-specific

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH - All APIs verified against UE 5.7 official documentation
- Architecture: HIGH - Patterns follow existing codebase conventions (component model, delegate binding) and verified UE5 APIs
- Pitfalls: HIGH - Volumetric fog ghosting and Lumen spotlight cost confirmed in official docs; component creation pattern confirmed in forums
- Audio asset pipeline: MEDIUM - MetaSound vs SoundCue recommendation based on official docs, but specific parameter API methods less thoroughly verified
- Flashlight-AI integration: MEDIUM - Mechanism for "flashlight attracts Wendigo" needs design-time decision; research provides options

**Research date:** 2026-02-12
**Valid until:** 2026-03-12 (stable UE5 APIs, unlikely to change)
