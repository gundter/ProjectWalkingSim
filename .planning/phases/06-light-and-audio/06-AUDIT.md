# Phase 6: Light and Audio - Performance Audit

**Date:** 2026-02-12
**Auditor:** Claude (automated)
**Scope:** All Phase 6 source files (Lighting/, Audio/) + Phase 6 additions to SereneCharacter and WendigoCharacter

---

## Critical Issues (must fix before phase complete)

### CRITICAL-1: Missing GetOwner() null check in UpdateHeartbeatProximity

- **File:** `Private/Audio/PlayerAudioComponent.cpp`, line 137-139
- **Severity:** Critical (potential crash)
- **Description:** `UpdateHeartbeatProximity()` calls `GetOwner()->GetActorLocation()` without a null check on `GetOwner()`. This is a timer callback that fires 4 times per second. If the owner is destroyed or pending kill while the timer is still active, this will crash.
- **Fix:** Add `AActor* Owner = GetOwner(); if (!Owner) return;` before the distance calculation.

### CRITICAL-2: MusicTensionSystem::EndPlay does not unbind delegates

- **File:** `Private/Audio/MusicTensionSystem.cpp`, lines 34-51
- **Severity:** Critical (dangling delegate, potential crash)
- **Description:** `EndPlay()` stops audio but never calls `RemoveDynamic` on the two delegates it bound in `BindToWendigoCharacter()`: `OnAlertLevelChanged` on SuspicionComponent and `OnBehaviorStateChanged` on WendigoCharacter. If the MusicTensionSystem component is destroyed before the Wendigo (e.g., streaming, respawn), the Wendigo will broadcast to a dangling delegate pointer, causing a crash.
- **Fix:** In `EndPlay()`, find the owning Wendigo, get its SuspicionComponent, and call `RemoveDynamic` on both delegates. MonsterAudioComponent already does this correctly (see lines 70-77 of its EndPlay) -- follow the same pattern.

### CRITICAL-3: Missing GetWorld() null check in PlayerAudioComponent::BeginPlay timer setup

- **File:** `Private/Audio/PlayerAudioComponent.cpp`, line 59
- **Severity:** Critical (potential crash)
- **Description:** `GetWorld()->GetTimerManager().SetTimer(...)` is called without checking if `GetWorld()` returns null. While unlikely during normal BeginPlay, this is unsafe if the component is activated in an unusual lifecycle state (e.g., editor preview, CDO construction). FlashlightComponent.cpp and other Phase 6 files correctly check `GetWorld()` before timer operations.
- **Fix:** Add `UWorld* World = GetWorld(); if (!World) return;` guard before timer setup.

---

## Warnings (should fix)

### WARN-1: Breathing crossfade uses single audio component (interrupts fade-out)

- **File:** `Private/Audio/MonsterAudioComponent.cpp`, lines 117-124
- **Severity:** Warning (audio artifact)
- **Description:** `TransitionBreathingSound` calls `FadeOut` on the breathing audio component, then immediately calls `SetSound` and `FadeIn` on the **same** component. The `SetSound` call interrupts the fade-out, so the crossfade effect is lost -- the old sound cuts abruptly to the new one instead of blending. A proper crossfade requires two audio components (one fading out, one fading in).
- **Impact:** Audible popping/cutting during state transitions instead of smooth crossfade.
- **Fix suggestion:** Use two `UAudioComponent` instances (ping-pong pattern): fade out the active one while fading in the other with the new sound. Swap which is "active" each transition. Alternatively, accept the current behavior as "quick swap with fade-in only" and rename the method to reflect that (lower priority fix -- playtest may reveal it's acceptable).

### WARN-2: LoadSynchronous called in timer callbacks (potential hitches)

- **File:** `Private/Audio/MonsterAudioComponent.cpp`, lines 111, 134, 174
- **File:** `Private/Audio/MusicTensionSystem.cpp`, lines 158, 166, 174
- **File:** `Private/Audio/AmbientAudioManager.cpp`, line 127
- **Severity:** Warning (performance hitch risk)
- **Description:** `TSoftObjectPtr::LoadSynchronous()` is called in timer callbacks and delegate handlers that execute during gameplay. If the referenced asset is not already loaded (first call, or after a GC), this will block the game thread to perform a synchronous disk load. For sounds, this can cause frame hitches (10-50ms depending on sound size and disk speed).
- **Impact:** Occasional single-frame stutters when sounds first play, especially on slower storage.
- **Fix suggestion:** Either (a) preload soft references during BeginPlay via `LoadSynchronous()` and cache the raw pointers, or (b) use `StreamableManager` async load with a callback. Option (a) is simplest for the current scope since there are only a handful of sounds per component. The BeginPlay-time loads in MusicTensionSystem (line 77) and AmbientAudioManager (line 35) already do this correctly -- extend the same pattern to the remaining timer/delegate paths.

### WARN-3: PlayerAudioComponent uses LogTemp instead of project log category

- **File:** `Private/Audio/PlayerAudioComponent.cpp`, line 31
- **Severity:** Warning (coding standards)
- **Description:** Uses `UE_LOG(LogTemp, ...)` instead of `LogSerene` or a dedicated `LogPlayerAudio` category. All other Phase 6 files use either `LogSerene` or define a file-local log category. `LogTemp` makes filtering difficult in production logs.
- **Fix:** Replace `LogTemp` with `LogSerene` (include `Core/SereneLogChannels.h`) or define `DEFINE_LOG_CATEGORY_STATIC(LogPlayerAudio, Log, All)`.

### WARN-4: AudioConstants.h missing CoreMinimal.h include

- **File:** `Public/Audio/AudioConstants.h`
- **Severity:** Warning (include hygiene)
- **Description:** The header uses `constexpr float` which is standard C++ and doesn't strictly need engine includes, but UE coding standards recommend every header include `CoreMinimal.h` for consistency and to ensure platform-specific type definitions are available. The file also lacks an `#include "CoreMinimal.h"` which means it depends on inclusion order (whoever includes AudioConstants.h must have already included CoreMinimal.h).
- **Note:** This is a minor hygiene issue. The `float` type is guaranteed by the standard. Nonetheless, for consistency with all other project headers, adding `#include "CoreMinimal.h"` is recommended.

### WARN-5: GetAllActorsOfClass for Wendigo lookup (O(n) world scan)

- **File:** `Private/Lighting/FlashlightComponent.cpp`, line 149-155 (`FindWendigo`)
- **File:** `Private/Audio/PlayerAudioComponent.cpp`, line 160-168 (`FindWendigo`)
- **File:** `Private/Audio/MusicTensionSystem.cpp`, line 106-111 (`BindToWendigo`)
- **Severity:** Warning (performance pattern)
- **Description:** Three separate components each call `UGameplayStatics::GetAllActorsOfClass` to find the single Wendigo. This iterates ALL actors in the world and allocates a `TArray<AActor*>` each call. While mitigated by caching (called once, then cached), it's a repeated pattern that creates temporary heap allocations.
- **Impact:** Low -- each call happens at most once (BeginPlay or first timer tick). But it's a scalability concern if more systems adopt this pattern.
- **Fix suggestion:** Consider a singleton/subsystem pattern (e.g., a `UWendigoSubsystem` that registers the Wendigo on spawn and provides `GetWendigo()` to all consumers). This is an architectural suggestion for a future cleanup pass, not a required fix.

---

## Notes (informational)

### NOTE-1: No Tick usage - excellent performance pattern

All Phase 6 components correctly disable Tick:
- `UFlashlightComponent`: `PrimaryComponentTick.bCanEverTick = false` + timer-based detection
- `UMonsterAudioComponent`: `PrimaryComponentTick.bCanEverTick = false` + delegate/timer driven
- `UMusicTensionSystem`: `PrimaryComponentTick.bCanEverTick = false` + delegate driven
- `AAmbientAudioManager`: `PrimaryActorTick.bCanEverTick = false` + timer driven
- `UPlayerAudioComponent`: `PrimaryComponentTick.bCanEverTick = false` + delegate/timer driven

This is the ideal pattern. Zero tick overhead from Phase 6 code.

### NOTE-2: Proper TObjectPtr usage throughout

All UPROPERTY pointer members use `TObjectPtr<>` (UE5 standard). No raw `UObject*` members found in UPROPERTY declarations. GC references are correctly tracked.

### NOTE-3: Proper UPROPERTY marking for GC

All `UAudioComponent*`, `USpotLightComponent*`, and other UObject-derived members are marked `UPROPERTY()` (even private ones without specifiers). This ensures GC doesn't collect them. No missing UPROPERTY annotations found.

### NOTE-4: Weak pointers used correctly for cross-actor references

- `FlashlightComponent::CachedWendigo` uses `TWeakObjectPtr<AWendigoCharacter>` - correct, avoids preventing GC
- `PlayerAudioComponent::CachedWendigo` uses `TWeakObjectPtr<AActor>` - correct
- `WendigoCharacter::WitnessedHidingSpot` uses `TWeakObjectPtr<AActor>` - correct

### NOTE-5: Timer cleanup in EndPlay is consistent

All components properly clear their timer handles in EndPlay and check GetWorld() for null (except CRITICAL-3). This prevents timer callbacks firing after component destruction.

### NOTE-6: No per-frame allocations

No `TArray` or `FString` construction found in timer callbacks or hot paths. The only `TArray` allocations are in `GetAllActorsOfClass` calls which are cached (one-time).

### NOTE-7: Naming conventions are correct

All classes follow UE naming: `U` prefix for components, `A` for actors, `E` for enums, `F` for structs/handles. UPROPERTY/UFUNCTION specifiers are appropriate. Blueprint categories are well-organized.

### NOTE-8: Thread safety is not a concern

All code runs on the game thread (timer callbacks, delegate handlers, BeginPlay/EndPlay). No async operations or multi-threaded access patterns. This is correct for UE audio components.

### NOTE-9: Chaos/ChaosEngineInterface.h include in PlayerAudioComponent.h

The `#include "Chaos/ChaosEngineInterface.h"` for `EPhysicalSurface` is functional but fragile. This header is part of the Chaos physics module and may be reorganized in future engine versions. A forward declaration won't work for enum usage in TMap key. The include works correctly for UE5.7 but should be monitored on engine upgrades.

### NOTE-10: MusicLayers as C-array vs TArray

`UMusicTensionSystem` uses `TObjectPtr<UAudioComponent> MusicLayers[3]` (C-style array) rather than `TArray<TObjectPtr<UAudioComponent>>`. This is fine -- the count is fixed at compile time and avoids heap allocation. The UPROPERTY reflection system handles fixed-size arrays correctly.

### NOTE-11: MonsterAudioComponent breathing crossfade - single component limitation

Related to WARN-1: The current single-component approach means `SetSound()` on a playing component will stop the current sound before starting the new one. This is documented in UE as expected behavior. The FadeOut call before SetSound is effectively overridden. This is a design compromise, not a bug -- true crossfade would need dual components.

---

## Summary

| Category | Count |
|----------|-------|
| Critical | 3 |
| Warning  | 5 |
| Notes    | 11 |

**Overall assessment:** Phase 6 code quality is **good**. The architecture is well-designed with zero tick overhead, proper delegate patterns, correct GC management, and clean separation of concerns. The three critical issues are straightforward null-check and delegate-cleanup fixes. The warnings are performance hygiene items (LoadSynchronous in callbacks, log category usage) that should be addressed but are not blocking.

**Fixes required before phase completion:**
1. Add GetOwner() null check in PlayerAudioComponent::UpdateHeartbeatProximity
2. Add delegate unbinding in MusicTensionSystem::EndPlay
3. Add GetWorld() null check in PlayerAudioComponent::BeginPlay timer setup
