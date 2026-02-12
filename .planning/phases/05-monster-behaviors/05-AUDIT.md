# Phase 5 Performance Audit

## Summary
- Critical issues: 2
- Warnings: 6
- Notes: 5

---

## Critical (must fix before Phase 6)

### C-1. `LineOfSightTo` called every tick during chase with no throttle
**File:** `Source/ProjectWalkingSim/Private/AI/Tasks/STT_ChasePlayer.cpp` line 104
**Issue:** `Controller.LineOfSightTo(Target)` performs a full collision trace every tick. During an active chase at 60+ FPS, this is a trace per frame. Unlike the AI controller's sight perception system (which uses an optimized sense query interval), this is a raw trace with no throttling.
**Impact:** Measurable CPU cost during chase, especially if multiple Wendigos chase simultaneously in future phases. LineOfSightTo also does not use the perception system's cached results -- it is a fresh trace each call.
**Fix:** Throttle the LOS check to every 0.15-0.25s using a timer in instance data. The LOS lost timer already accumulates DeltaTime, so a brief polling interval will not meaningfully affect responsiveness (the 3-second timeout gives ample margin). Between polls, carry forward the last-known LOS result.

```cpp
// In instance data:
float LOSCheckTimer = 0.0f;
bool bLastKnownLOS = true;

// In Tick:
InstanceData.LOSCheckTimer += DeltaTime;
bool bCanSeePlayer = InstanceData.bLastKnownLOS;
if (InstanceData.LOSCheckTimer >= 0.2f)
{
    bCanSeePlayer = Controller.LineOfSightTo(Target);
    InstanceData.bLastKnownLOS = bCanSeePlayer;
    InstanceData.LOSCheckTimer = 0.0f;
}
```

### C-2. Division by zero possible in `ProcessSightStimulus` if `VisibilityThreshold >= 1.0`
**File:** `Source/ProjectWalkingSim/Private/AI/SuspicionComponent.cpp` line 20-21
**Issue:** The effective visibility normalization divides by `(1.0f - VisibilityThreshold)`. The `VisibilityThreshold` is an `EditAnywhere` UPROPERTY with no `ClampMax` meta. If a designer sets it to 1.0 or above, this produces a division by zero or negative divisor, causing NaN/Inf propagation into suspicion values.
**Impact:** Suspicion system would break entirely for that AI instance. NaN comparison in `UpdateAlertLevel` would produce undefined alert state behavior.
**Fix:** Add `ClampMax = "0.99"` meta to the `VisibilityThreshold` UPROPERTY, and add a runtime guard:

```cpp
// In SuspicionComponent.h, line 95:
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Suspicion|Tuning",
    meta = (ClampMin = "0.0", ClampMax = "0.99"))
float VisibilityThreshold = AIConstants::VisibilityThreshold;

// In ProcessSightStimulus, as a safety net:
const float Divisor = FMath::Max(1.0f - VisibilityThreshold, KINDA_SMALL_NUMBER);
const float EffectiveVisibility = (PlayerVisibilityScore - VisibilityThreshold) / Divisor;
```

---

## Warnings (should fix)

### W-1. `GetCurrentlyPerceivedActors` allocates a TArray every tick in AI controller
**File:** `Source/ProjectWalkingSim/Private/AI/WendigoAIController.cpp` line 107
**Issue:** `GetCurrentlyPerceivedActors` creates a new TArray on the stack each tick. For a single Wendigo this is minor, but with multiple AI instances, this adds up to per-frame heap allocations (TArray small buffer optimization may help for very small counts, but is not guaranteed).
**Fix:** Cache the TArray as a member or reserve capacity once. Alternatively, iterate `GetKnownPerceivedActors` and filter by sense, which avoids the intermediate array.

### W-2. `GetCurrentlyPerceivedActors` also called in `OnPlayerHidingStateChanged`
**File:** `Source/ProjectWalkingSim/Private/AI/WendigoAIController.cpp` line 250-251
**Issue:** Same TArray allocation pattern. This is event-driven so less frequent, but still allocates per call. Additionally, the perception system's `GetCurrentlyPerceivedActors` internally iterates all perceived actors -- for a hiding event callback this is acceptable but worth noting.
**Severity:** Low. Event-driven frequency makes this a minor concern.

### W-3. `STT_InvestigateLocation::ExitState` unconditionally sets BehaviorState to Patrol
**File:** `Source/ProjectWalkingSim/Private/AI/Tasks/STT_InvestigateLocation.cpp` line 151
**Issue:** If the investigation is interrupted by a transition to Chase (alert level reached Alert during investigation), ExitState will momentarily set BehaviorState to Patrol before the Chase task's EnterState sets it to Chasing. This causes a spurious `OnBehaviorStateChanged` broadcast (Investigating -> Patrol -> Chasing). Any listener (UI, audio) would see an incorrect Patrol state flash.
**Fix:** Only reset to Patrol if the transition is not going to a higher-priority state. The simplest approach: remove the `SetBehaviorState(Patrol)` from ExitState entirely. The entering task is responsible for setting its own behavior state, and the Wendigo should not briefly flash back to Patrol mid-escalation. If patrol reset is needed for the natural "investigation completed" path, guard it:

```cpp
// Only reset to patrol if investigation completed successfully (not interrupted by escalation)
if (Transition.CurrentRunStatus == EStateTreeRunStatus::Succeeded)
{
    Wendigo->SetBehaviorState(EWendigoBehaviorState::Patrol);
}
```

### W-4. `STT_GrabAttack` uses `ConsoleCommand("RestartLevel")` which is not deterministic
**File:** `Source/ProjectWalkingSim/Private/AI/Tasks/STT_GrabAttack.cpp` line 83
**Issue:** `ConsoleCommand("RestartLevel")` is a deferred console command. The task returns `Succeeded` immediately after issuing it, but the level restart happens asynchronously. During the frames between the command and actual restart, ExitState runs and re-enables player input, potentially allowing a brief window of player movement during the restart transition. The code comment notes this is demo-scope and will be replaced in Phase 8 -- this is acceptable for now but should be tracked.
**Impact:** Low for demo. The input re-enable in ExitState is the correct safety pattern; the timing window is typically sub-frame.

### W-5. AI controller tick runs even when no pawn is possessed
**File:** `Source/ProjectWalkingSim/Private/AI/WendigoAIController.cpp` line 89-98
**Issue:** The controller's Tick has `bCanEverTick = true` set in the constructor. The early-out at line 95 (`if (!WendigoChar) return`) handles the no-pawn case, but the tick still fires. Between spawn and possession, or after unpossession, Tick runs with nothing to do.
**Fix:** Disable tick until OnPossess and re-disable on OnUnPossess. This is a minor overhead saving but cleaner:

```cpp
void AWendigoAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    SetActorTickEnabled(true);
    // ...
}
```

### W-6. `WendigoSpawnPoint::SpawnWendigo` does not null-check GetWorld()
**File:** `Source/ProjectWalkingSim/Private/AI/WendigoSpawnPoint.cpp` line 35
**Issue:** `GetWorld()->SpawnActor<>()` dereferences GetWorld() without checking for null. While GetWorld() is nearly always valid for placed actors, calling SpawnWendigo from a non-world context (e.g., a CDO or during shutdown) would crash.
**Fix:** Add a null check: `UWorld* World = GetWorld(); if (!World) return nullptr;`

---

## Notes (minor/style)

### N-1. Debug logging in AI controller tick should be compiled out or use Verbose
**File:** `Source/ProjectWalkingSim/Private/AI/WendigoAIController.cpp` lines 138-141
**Issue:** The 1-second periodic sight debug log uses `UE_LOG(LogSerene, Log, ...)`. In a shipping build this will still execute the string format even if the log category is suppressed. Consider `UE_LOG(LogSerene, Verbose, ...)` or wrapping in `#if !UE_BUILD_SHIPPING`.
**Severity:** Cosmetic/log noise. The timer throttle already mitigates most overhead.

### N-2. Gameplay tags declared but not used in Phase 5 C++ code
**File:** `Source/ProjectWalkingSim/Public/Tags/SereneTags.h` lines 41-57
**Issue:** The AI alert level, stimulus type, behavior state, and spawn zone tags are declared and defined but never referenced in any Phase 5 C++ code. They may be used in Blueprint State Tree assets or planned for Phase 6. If so, this is fine -- just noting for completeness.
**Severity:** No runtime cost (tags are registered on startup regardless). Intentional forward declaration is a valid pattern.

### N-3. `CameraBlendTime` property in STT_GrabAttack is unused
**File:** `Source/ProjectWalkingSim/Public/AI/Tasks/STT_GrabAttack.h` line 78
**Issue:** `CameraBlendTime` is declared with `EditAnywhere` but never read in EnterState, Tick, or ExitState. The comment says "if used by future cinematic system." This is a placeholder property exposed in the editor with no current effect.
**Fix:** Either remove it until Phase 8 needs it, or add a comment `// Reserved for Phase 8` to the meta. As-is, a designer might change it and be confused that nothing happens.

### N-4. `WendigoCharacter::WitnessedHidingSpot` is public UPROPERTY with TWeakObjectPtr
**File:** `Source/ProjectWalkingSim/Public/AI/WendigoCharacter.h` line 59
**Issue:** `TWeakObjectPtr<AActor>` is the correct pointer type here (avoids preventing GC on the hiding spot). However, it is exposed as a public `UPROPERTY(BlueprintReadOnly)`. `TWeakObjectPtr` in UPROPERTY is supported but has nuanced Blueprint interop -- Blueprint reads will see the raw weak pointer wrapper. The getter/setter pattern used for LastKnownPlayerLocation would be more consistent here.
**Severity:** Style consistency. Functionally correct.

### N-5. State Tree task UPROPERTY fields on structs are public by default
**File:** All STT_*.h files
**Issue:** The `UPROPERTY(EditAnywhere)` fields on the State Tree task structs (ChaseSpeed, LOSLostTimeout, GrabRange, etc.) are at struct-default public visibility. This is correct and expected for State Tree task structs -- the editor needs public access to serialize and display them. Noting explicitly that this is not a standards violation despite the general preference for private/protected on UCLASS members.

---

## Performance Profile Summary

| Component | Tick Cost | Notes |
|---|---|---|
| WendigoAIController::Tick | Medium | Per-frame GetCurrentlyPerceivedActors + sight processing. Necessary for continuous suspicion accumulation. |
| STT_ChasePlayer::Tick | **High** | Per-frame LineOfSightTo trace + MoveStatus check. LOS should be throttled (C-1). |
| STT_SearchArea::Tick | Low | Timer increments + occasional MoveToLocation. NavMesh queries only on point transitions. |
| STT_ReturnToNearestWaypoint::Tick | Minimal | Single MoveStatus check per tick. |
| STT_GrabAttack::Tick | Minimal | Timer increment only. |
| STT_InvestigateLocation::Tick | Low | MoveStatus check + timer increment. |
| SuspicionComponent | None (tickless) | Driven by AI controller, no autonomous tick. Correct design. |
| DoorActor::Tick | Minimal | Self-disabling tick (bStartWithTickEnabled=false). Only active during animation. Correct design. |
| WendigoSpawnPoint | None | bCanEverTick=false. Correct. |

## Files Reviewed
- `Source/ProjectWalkingSim/Public/AI/MonsterAITypes.h`
- `Source/ProjectWalkingSim/Private/AI/MonsterAITypes.cpp`
- `Source/ProjectWalkingSim/Public/AI/WendigoCharacter.h`
- `Source/ProjectWalkingSim/Private/AI/WendigoCharacter.cpp`
- `Source/ProjectWalkingSim/Public/AI/SuspicionComponent.h`
- `Source/ProjectWalkingSim/Private/AI/SuspicionComponent.cpp`
- `Source/ProjectWalkingSim/Public/AI/WendigoAIController.h`
- `Source/ProjectWalkingSim/Private/AI/WendigoAIController.cpp`
- `Source/ProjectWalkingSim/Public/AI/WendigoSpawnPoint.h`
- `Source/ProjectWalkingSim/Private/AI/WendigoSpawnPoint.cpp`
- `Source/ProjectWalkingSim/Public/AI/Tasks/STT_ChasePlayer.h`
- `Source/ProjectWalkingSim/Private/AI/Tasks/STT_ChasePlayer.cpp`
- `Source/ProjectWalkingSim/Public/AI/Tasks/STT_SearchArea.h`
- `Source/ProjectWalkingSim/Private/AI/Tasks/STT_SearchArea.cpp`
- `Source/ProjectWalkingSim/Public/AI/Tasks/STT_ReturnToNearestWaypoint.h`
- `Source/ProjectWalkingSim/Private/AI/Tasks/STT_ReturnToNearestWaypoint.cpp`
- `Source/ProjectWalkingSim/Public/AI/Tasks/STT_GrabAttack.h`
- `Source/ProjectWalkingSim/Private/AI/Tasks/STT_GrabAttack.cpp`
- `Source/ProjectWalkingSim/Public/AI/Tasks/STT_InvestigateLocation.h`
- `Source/ProjectWalkingSim/Private/AI/Tasks/STT_InvestigateLocation.cpp`
- `Source/ProjectWalkingSim/Public/AI/Conditions/STC_StimulusType.h`
- `Source/ProjectWalkingSim/Private/AI/Conditions/STC_StimulusType.cpp`
- `Source/ProjectWalkingSim/Public/Interaction/DoorActor.h`
- `Source/ProjectWalkingSim/Private/Interaction/DoorActor.cpp`
- `Source/ProjectWalkingSim/Public/Tags/SereneTags.h`
- `Source/ProjectWalkingSim/Private/Tags/SereneTags.cpp`
- `Source/ProjectWalkingSim/Public/AI/PatrolRouteActor.h` (reference for GetWaypoint API)
