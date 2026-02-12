---
phase: 05-monster-behaviors
plan: 04
subsystem: ai
tags: [wendigo, spawn-point, hiding-detection, door-ai, perception-delegate]

# Dependency graph
requires:
  - phase: 05-01
    provides: "EWendigoBehaviorState, chase/search persistent state, WitnessedHidingSpot on WendigoCharacter"
  - phase: 04
    provides: "WendigoAIController with sight/hearing perception pipeline, PatrolRouteActor, SuspicionComponent"
  - phase: 03
    provides: "HidingComponent with OnHidingStateChanged delegate, EHidingState enum"
provides:
  - "AWendigoSpawnPoint: zone-based spawn with random patrol route assignment"
  - "WendigoCharacter::SetPatrolRoute: runtime patrol route assignment"
  - "ADoorActor::OpenForAI: AI-compatible door opening respecting lock state"
  - "AWendigoAIController: witnessed-hiding detection via perception + delegate timing"
  - "AWendigoAIController: continuous LastKnownPlayerLocation updates in Tick"
affects: ["05-05 (search behavior uses witnessed hiding)", "06 (lighting affects visibility for witnessed hiding)", "08 (polish/tuning)"]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Delegate binding on first perception event (not constructor/BeginPlay)"
    - "Cross-module delegate subscription (AI -> Hiding) via forward declaration"
    - "Protected member promotion for cross-class access (DoorActor state fields)"

key-files:
  created:
    - "Source/ProjectWalkingSim/Public/AI/WendigoSpawnPoint.h"
    - "Source/ProjectWalkingSim/Private/AI/WendigoSpawnPoint.cpp"
  modified:
    - "Source/ProjectWalkingSim/Public/AI/WendigoAIController.h"
    - "Source/ProjectWalkingSim/Private/AI/WendigoAIController.cpp"
    - "Source/ProjectWalkingSim/Public/AI/WendigoCharacter.h"
    - "Source/ProjectWalkingSim/Public/Interaction/DoorActor.h"
    - "Source/ProjectWalkingSim/Private/Interaction/DoorActor.cpp"

key-decisions:
  - "Bind player hiding delegate on first sight detection, not in constructor or BeginPlay"
  - "Forward declare EHidingState to avoid circular AI-Hiding header dependency"
  - "SetPatrolRoute method (Option B) preserves EditInstanceOnly for level designers"
  - "DoorActor state fields promoted from private to protected for OpenForAI access"

patterns-established:
  - "Cross-module delegate binding: subscribe on first interaction, not at init"
  - "Runtime spawned AI: SpawnPoint assigns route via setter, not UPROPERTY"

# Metrics
duration: 8min
completed: 2026-02-12
---

# Phase 5 Plan 4: Spawn Points, Witnessed Hiding, and AI Door Opening Summary

**WendigoSpawnPoint for zone-based AI spawning, witnessed-hiding detection via perception-delegate timing, and AI-compatible door opening**

## Performance

- **Duration:** ~8 min
- **Started:** 2026-02-12T14:58:23Z
- **Completed:** 2026-02-12T15:05:54Z
- **Tasks:** 2
- **Files modified:** 7

## Accomplishments
- AWendigoSpawnPoint spawns Wendigos with random patrol route from zone-specific route arrays
- AI controller detects when player enters hiding while being seen and records WitnessedHidingSpot
- AI controller continuously updates LastKnownPlayerLocation every tick while player is visible
- DoorActor::OpenForAI lets AI open unlocked doors using same swing-direction logic as players
- Locked doors remain impassable to AI (no bypass)

## Task Commits

Each task was committed atomically:

1. **Task 1: Create WendigoSpawnPoint and extend DoorActor** - `9fb454f` (feat) -- pre-committed by 05-02 wave execution
2. **Task 2: Extend WendigoAIController with witnessed-hiding detection** - `cf34fcc` (feat)

**Plan metadata:** pending (docs: complete plan)

## Files Created/Modified
- `Source/ProjectWalkingSim/Public/AI/WendigoSpawnPoint.h` - Spawn point actor with zone-based patrol route arrays
- `Source/ProjectWalkingSim/Private/AI/WendigoSpawnPoint.cpp` - SpawnWendigo implementation with random route selection
- `Source/ProjectWalkingSim/Public/AI/WendigoCharacter.h` - Added SetPatrolRoute for runtime route assignment
- `Source/ProjectWalkingSim/Public/AI/WendigoAIController.h` - Added hiding detection members, forward decl of EHidingState
- `Source/ProjectWalkingSim/Private/AI/WendigoAIController.cpp` - BindToPlayerDelegates, OnPlayerHidingStateChanged, LastKnownPlayerLocation updates
- `Source/ProjectWalkingSim/Public/Interaction/DoorActor.h` - OpenForAI method, promoted state fields to protected
- `Source/ProjectWalkingSim/Private/Interaction/DoorActor.cpp` - OpenForAI implementation with lock check and swing direction

## Decisions Made
- **Bind player hiding delegate on first sight, not at init** -- Player may not exist at controller init; binding on first sight ensures the player's HidingComponent is available (Pitfall 7 from research).
- **Forward declare EHidingState in AI header** -- Full include of HidingTypes.h only in .cpp; avoids circular AI-Hiding dependency.
- **SetPatrolRoute method (Option B)** -- Preserves EditInstanceOnly restriction on PatrolRoute for level designers while enabling runtime assignment from spawn system.
- **DoorActor state fields promoted to protected** -- bIsOpen, CurrentAngle, TargetAngle, OpenDirection moved from private to protected so OpenForAI can access them without friend declaration.
- **Task 1 pre-committed by wave execution** -- Plans 05-02/03/04 ran in parallel wave; 05-02 implemented WendigoSpawnPoint, SetPatrolRoute, and OpenForAI as part of its scope (same pattern as 04-04/04-05 overlap).

## Deviations from Plan

None - plan executed exactly as written. Task 1 was pre-committed by wave-parallel execution of 05-02 with identical implementation.

## Issues Encountered
- Task 1 was already committed in `9fb454f` (05-02 wave execution). Writes produced identical content so git detected no changes. Task 2 was the unique contribution of this plan.

## User Setup Required
None - no external service configuration required.

## Next Phase Readiness
- Spawn points ready for level placement with per-zone patrol routes
- Witnessed hiding detection enables search behavior (05-05) to check hiding spots the Wendigo saw the player enter
- AI door opening prevents Wendigo from getting stuck during chase sequences
- All Phase 5 infrastructure (data layer, chase, grab, spawn, hiding detection) ready for search behavior plan

---
*Phase: 05-monster-behaviors*
*Completed: 2026-02-12*
