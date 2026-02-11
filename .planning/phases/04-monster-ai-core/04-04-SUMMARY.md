---
phase: 04-monster-ai-core
plan: 04
subsystem: ai
tags: [state-tree, patrol, waypoints, ai-controller, navigation, ustruct-task]

# Dependency graph
requires:
  - phase: 04-monster-ai-core (04-02)
    provides: "AWendigoCharacter with PatrolRoute ref, APatrolRouteActor stub"
  - phase: 04-monster-ai-core (04-03)
    provides: "AWendigoAIController with StateTreeAIComponent and perception"
provides:
  - "APatrolRouteActor: full waypoint container with loop/ping-pong, editor visualization, MakeEditWidget"
  - "FSTT_PatrolMoveToWaypoint: State Tree task for MoveToLocation-based waypoint navigation"
  - "FSTT_PatrolIdle: State Tree task for random-duration idle with look-around behavior"
affects: [05-monster-behaviors, 08-demo-polish]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "FStateTreeTaskCommonBase USTRUCT pattern for custom C++ State Tree tasks"
    - "TStateTreeExternalDataHandle<AAIController> for accessing controller from tasks"
    - "Instance data structs for per-activation runtime state"
    - "MakeEditWidget meta on TArray<FVector> for viewport-editable waypoints"
    - "Editor-only Tick with DrawDebugHelpers for design-time visualization"

key-files:
  created:
    - "Source/ProjectWalkingSim/Public/AI/Tasks/STT_PatrolMoveToWaypoint.h"
    - "Source/ProjectWalkingSim/Private/AI/Tasks/STT_PatrolMoveToWaypoint.cpp"
    - "Source/ProjectWalkingSim/Public/AI/Tasks/STT_PatrolIdle.h"
    - "Source/ProjectWalkingSim/Private/AI/Tasks/STT_PatrolIdle.cpp"
  modified:
    - "Source/ProjectWalkingSim/Public/AI/PatrolRouteActor.h"
    - "Source/ProjectWalkingSim/Private/AI/PatrolRouteActor.cpp"
    - "Source/ProjectWalkingSim/Public/AI/WendigoAIController.h"
    - "Source/ProjectWalkingSim/Private/AI/WendigoAIController.cpp"

key-decisions:
  - "Clamp waypoint index instead of modulo wrap (safer boundary behavior)"
  - "Mutable PingPongDirection on PatrolRouteActor for const-correct GetNextWaypointIndex"
  - "AlreadyAtGoal returns Succeeded immediately (edge case: AI spawned on waypoint)"
  - "Look-around at 40% idle, clear at 75% (creates observeable scan pattern for player)"

patterns-established:
  - "STT_ prefix for State Tree task structs (FSTT_PatrolMoveToWaypoint)"
  - "Instance data struct naming: FSTT_[TaskName]InstanceData"
  - "Link/EnterState/Tick/ExitState lifecycle for all State Tree tasks"
  - "Editor-only visualization via WITH_EDITOR + IsEditorWorld() guard in Tick"

# Metrics
duration: 6min
completed: 2026-02-11
---

# Phase 4 Plan 04: Patrol Route and State Tree Tasks Summary

**Patrol waypoint system with editor visualization and two custom State Tree tasks (MoveToWaypoint + Idle with look-around)**

## Performance

- **Duration:** ~6 min
- **Started:** 2026-02-11T16:00:54Z
- **Completed:** 2026-02-11T16:06:34Z
- **Tasks:** 2
- **Files modified:** 8 (4 created, 4 modified)

## Accomplishments

- Expanded PatrolRouteActor from stub to full waypoint container with loop/ping-pong modes, editor debug visualization (green spheres/lines, blue loop-back), and MakeEditWidget for viewport drag handles
- Created FSTT_PatrolMoveToWaypoint State Tree task that uses AAIController::MoveToLocation to navigate between patrol waypoints, advancing the index on arrival
- Created FSTT_PatrolIdle State Tree task with randomized 3-6s duration and deliberate look-around behavior (glance at 40%, return forward at 75%)

## Task Commits

Each task was committed atomically:

1. **Task 1: PatrolRouteActor** - `b99d36c` (feat)
2. **Task 2: Patrol State Tree tasks** - `5806c94` (feat)

**Plan metadata:** [pending] (docs: complete plan)

## Files Created/Modified

- `Source/ProjectWalkingSim/Public/AI/PatrolRouteActor.h` - Expanded with bLoopRoute, GetNextWaypointIndex, MakeEditWidget, editor billboard, Tick override
- `Source/ProjectWalkingSim/Private/AI/PatrolRouteActor.cpp` - Full implementation: loop/ping-pong index, editor debug spheres/lines
- `Source/ProjectWalkingSim/Public/AI/Tasks/STT_PatrolMoveToWaypoint.h` - State Tree task struct and instance data for waypoint navigation
- `Source/ProjectWalkingSim/Private/AI/Tasks/STT_PatrolMoveToWaypoint.cpp` - MoveToLocation, GetMoveStatus polling, waypoint index advancement
- `Source/ProjectWalkingSim/Public/AI/Tasks/STT_PatrolIdle.h` - State Tree task struct and instance data for patrol idle/scan
- `Source/ProjectWalkingSim/Private/AI/Tasks/STT_PatrolIdle.cpp` - Random duration, focal point look-around, cleanup on exit
- `Source/ProjectWalkingSim/Public/AI/WendigoAIController.h` - Renamed Instigator parameter to NoiseInstigator
- `Source/ProjectWalkingSim/Private/AI/WendigoAIController.cpp` - Renamed Instigator parameter to NoiseInstigator

## Decisions Made

- **Clamp vs modulo for GetWaypoint:** Changed from modulo wrapping (in stub) to FMath::Clamp for safer boundary behavior. GetNextWaypointIndex handles the advancement logic explicitly.
- **Mutable PingPongDirection:** Made mutable so GetNextWaypointIndex can be const while tracking ping-pong direction state. This is logically a query operation.
- **AlreadyAtGoal handling:** If MoveToLocation returns AlreadyAtGoal (AI spawned on waypoint), immediately advance index and return Succeeded rather than returning Running with nothing to poll.
- **Look-around timing (40%/75%):** At 40% through idle the Wendigo glances to a random side; at 75% it returns forward. This creates observable scan patterns players can learn.
- **CreateEditorOnlyDefaultSubobject for billboard:** Uses editor-only subobject pattern so billboard component is stripped from cooked builds.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Fixed AActor::Instigator parameter shadowing in WendigoAIController**
- **Found during:** Task 1 (compilation check)
- **Issue:** `ProcessHearingPerception(AActor* Instigator, ...)` parameter name shadows `AActor::Instigator` member, causing C4458 warning-as-error
- **Fix:** Renamed parameter to `NoiseInstigator` in both .h and .cpp
- **Files modified:** WendigoAIController.h, WendigoAIController.cpp
- **Verification:** Compilation succeeds with -WarningsAsErrors
- **Committed in:** b99d36c (Task 1 commit)

---

**Total deviations:** 1 auto-fixed (1 blocking)
**Impact on plan:** Pre-existing compilation error from previous plan required parameter rename. No scope creep.

## Issues Encountered

- The UE5.7 `DrawDebugHelpers.h` required as an explicit include for editor visualization (not transitively included).
- State Tree task headers need `StateTreeLinker.h` and `StateTreeExecutionContext.h` for external data handles and instance data access -- not included transitively from `StateTreeTaskBase.h`.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- Patrol route and tasks are ready for State Tree asset authoring (editor-side wiring)
- Tasks will appear in the State Tree editor as "Patrol: Move To Waypoint" and "Patrol: Idle / Look Around"
- A NavMesh must be placed in the level and configured for the Wendigo's tall agent profile before patrol navigation works at runtime
- State Tree asset + NavMeshBoundsVolume + PatrolRouteActor placement needed in a future plan
- Ready for Phase 5 tasks (investigate, chase) which follow the same FSTT_ pattern

---
*Phase: 04-monster-ai-core*
*Completed: 2026-02-11*
