---
phase: 06-light-and-audio
plan: 04
subsystem: audio
tags: [footsteps, heartbeat, proximity, MetaSound, UAudioComponent, timer, EPhysicalSurface]

# Dependency graph
requires:
  - phase: 01-foundation
    provides: FootstepComponent with OnFootstep delegate (EPhysicalSurface, float)
  - phase: 04-monster-ai-core
    provides: AWendigoCharacter for proximity distance checks
provides:
  - UPlayerAudioComponent with surface-aware footstep playback
  - Heartbeat proximity system driven by MetaSound Intensity parameter
  - Timer-based (4Hz) Wendigo proximity check with no Tick overhead
affects: [08-demo-polish]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Timer-based audio parameter updates (4Hz) instead of per-frame Tick"
    - "2D non-spatialized audio for player-internal sounds (bIsUISound=true)"
    - "TSoftObjectPtr<USoundBase> for deferred sound asset loading"
    - "TWeakObjectPtr for safe cross-system actor references"

key-files:
  created:
    - Source/ProjectWalkingSim/Public/Audio/PlayerAudioComponent.h
    - Source/ProjectWalkingSim/Private/Audio/PlayerAudioComponent.cpp
  modified: []

key-decisions:
  - "PlaySound2D for player footsteps (non-spatialized, player hears own steps)"
  - "bIsUISound=true for heartbeat (non-spatialized player-internal sound)"
  - "TWeakObjectPtr<AActor> for CachedWendigo (handles respawn/destroy safely)"
  - "No compile-time dependency on AudioConstants.h (enables Wave 1 parallel execution)"

patterns-established:
  - "Player audio component pattern: event-driven + timer-based, zero Tick"
  - "Footstep delegate consumer pattern: bind in BeginPlay, unbind in EndPlay"

# Metrics
duration: 4min
completed: 2026-02-12
---

# Phase 6 Plan 04: Player Audio Component Summary

**Surface-aware footstep playback via OnFootstep delegate and heartbeat proximity system with 4Hz timer-driven MetaSound Intensity parameter toward Wendigo**

## Performance

- **Duration:** ~4 min
- **Started:** 2026-02-12T19:36:14Z
- **Completed:** 2026-02-12T19:40:14Z
- **Tasks:** 2
- **Files created:** 2

## Accomplishments
- PlayerAudioComponent listens to FootstepComponent::OnFootstep and plays surface-type-aware sounds via TMap<EPhysicalSurface, TSoftObjectPtr<USoundBase>>
- Heartbeat proximity system checks distance to nearest Wendigo at 4Hz on a timer, maps inverse distance to 0-1 Intensity MetaSound parameter
- Zero Tick overhead -- footsteps are event-driven, heartbeat uses FTimerHandle
- Both footstep and heartbeat audio are 2D non-spatialized (player hears own steps and heartbeat non-directionally)

## Task Commits

Each task was committed atomically:

1. **Task 1: Create PlayerAudioComponent header** - `0cc5e24` (feat)
2. **Task 2: Implement PlayerAudioComponent** - `a6d2e5b` (feat)

## Files Created/Modified
- `Source/ProjectWalkingSim/Public/Audio/PlayerAudioComponent.h` - UPlayerAudioComponent class with footstep and heartbeat properties, HandleFootstep delegate handler, UpdateHeartbeatProximity timer callback
- `Source/ProjectWalkingSim/Private/Audio/PlayerAudioComponent.cpp` - Full implementation: BeginPlay binds to OnFootstep, creates HeartbeatAudioComp (2D), starts 4Hz timer; HandleFootstep does surface-type lookup and PlaySound2D; UpdateHeartbeatProximity does inverse-distance mapping to SetFloatParameter

## Decisions Made
- **PlaySound2D for player footsteps:** Player hears their own footsteps non-spatially. The FootstepComponent's Volume parameter already accounts for sprint/crouch multipliers, so we just multiply by FootstepBaseVolume.
- **bIsUISound=true for heartbeat:** Heartbeat is a player-internal experience, not a world sound. Non-spatialized ensures it stays centered regardless of camera orientation.
- **TWeakObjectPtr<AActor> for CachedWendigo:** Safe reference that auto-invalidates if Wendigo is destroyed/respawned. FindWendigo() re-caches on next proximity check.
- **No AudioConstants.h dependency:** Inline UPROPERTY defaults (0.6f, 3000.0f, 500.0f, 4.0f) match AudioConstants values but don't require the include, enabling Wave 1 parallel compilation.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

- **Parallel wave linker error:** Build initially showed linker errors for `UMonsterAudioComponent` symbols (header existed from Plan 06-02 wave but .cpp not yet committed). On re-inspection, the .cpp file was present on disk from a parallel agent but untracked. Subsequent build succeeded. No code changes needed for PlayerAudioComponent itself.

## User Setup Required

None - no external service configuration required. Sound assets (footstep SoundWaves, heartbeat MetaSound) will be created in Phase 8 (Demo Polish) and assigned to the component's TSoftObjectPtr properties in the editor.

## Next Phase Readiness
- PlayerAudioComponent ready for Blueprint attachment to SereneCharacter
- Sound assets needed: surface-specific footstep sounds, heartbeat MetaSound with Intensity parameter
- Pairs with MonsterAudioComponent (Plan 06-02) for the "dual proximity cue" system from CONTEXT.md

---
*Phase: 06-light-and-audio*
*Completed: 2026-02-12*
