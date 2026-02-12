---
phase: 05-monster-behaviors
plan: 02
subsystem: ai
tags: [state-tree, chase, search, patrol, navmesh, line-of-sight, movement]

# Dependency graph
requires:
  - phase: 04-monster-ai-core
    provides: "AAIController, State Tree tasks pattern, PatrolRouteActor, SuspicionComponent"
  - phase: 05-01
    provides: "EWendigoBehaviorState, AIConstants chase/search/grab values, WendigoCharacter persistent state"
provides:
  - "FSTT_ChasePlayer State Tree task with MoveToActor pursuit, LOS timer, grab range check"
  - "FSTT_SearchArea State Tree task with last-known + random NavMesh point navigation and linger"
  - "FSTT_ReturnToNearestWaypoint State Tree task for post-search patrol resumption"
affects: [05-04-spawn-system, 05-05-state-tree-wiring]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "MoveToActor for continuous pursuit (not repeated MoveToLocation)"
    - "GetRandomReachablePointInRadius for search area NavMesh point generation"
    - "DistSquared for nearest-waypoint lookup (avoids sqrt)"
    - "All movement tasks restore AIConstants::WendigoWalkSpeed in ExitState to prevent speed leaks"

key-files:
  created:
    - Source/ProjectWalkingSim/Public/AI/Tasks/STT_ChasePlayer.h
    - Source/ProjectWalkingSim/Private/AI/Tasks/STT_ChasePlayer.cpp
    - Source/ProjectWalkingSim/Public/AI/Tasks/STT_SearchArea.h
    - Source/ProjectWalkingSim/Private/AI/Tasks/STT_SearchArea.cpp
    - Source/ProjectWalkingSim/Public/AI/Tasks/STT_ReturnToNearestWaypoint.h
    - Source/ProjectWalkingSim/Private/AI/Tasks/STT_ReturnToNearestWaypoint.cpp
  modified: []

key-decisions:
  - "MoveToActor for chase, not MoveToLocation -- MoveToActor auto-updates destination as player moves"
  - "LOS check via AAIController::LineOfSightTo, not custom raycasts"
  - "Grab range requires LOS (no blind grabs through walls)"
  - "SearchArea uses SearchRadius=600cm max for GetRandomReachablePointInRadius reliability"
  - "SearchArea ExitState wipes LastKnownPlayerLocation and WitnessedHidingSpot (clean slate after search)"

patterns-established:
  - "Hunt cycle: Chase (575cm/s) -> LOS lost -> Search (180cm/s) -> Timeout -> ReturnToWaypoint (150cm/s) -> Patrol"
  - "Periodic look-around during SearchArea linger (random yaw offset every ~2s, similar to PatrolIdle)"

# Metrics
duration: 8min
completed: 2026-02-12
---

# Phase 5 Plan 02: Hunt-Cycle State Tree Tasks Summary

**Three State Tree tasks forming the complete predator hunt cycle: Chase (MoveToActor with LOS timer), Search (last-known + random NavMesh points), ReturnToNearestWaypoint (DistSquared nearest waypoint for patrol resume)**

## Performance

- **Duration:** ~8 min
- **Started:** 2026-02-12T14:58:28Z
- **Completed:** 2026-02-12T15:06:40Z
- **Tasks:** 2/2
- **Files created:** 6

## Accomplishments
- Created FSTT_ChasePlayer: pursues player at 575 cm/s via MoveToActor, monitors LOS (returns Failed after 3s timeout), checks grab range (returns Succeeded within 150cm + LOS), re-issues MoveToActor if path goes idle mid-chase
- Created FSTT_SearchArea: navigates to LastKnownPlayerLocation then 2-3 random NavMesh points, lingers at each with periodic look-around head turns, times out after MaxSearchDuration (18s)
- Created FSTT_ReturnToNearestWaypoint: iterates all waypoints by DistSquared, sets CurrentWaypointIndex for seamless patrol resumption, navigates at patrol speed

## Task Commits

Each task was committed atomically:

1. **Task 1: Create STT_ChasePlayer task** - `9fb454f` (feat) [from prior session]
2. **Task 2: Create STT_SearchArea and STT_ReturnToNearestWaypoint tasks** - `20a1ebb` (feat)

## Files Created/Modified
- `Source/ProjectWalkingSim/Public/AI/Tasks/STT_ChasePlayer.h` - Chase task with LOS timer, grab range, chase speed properties
- `Source/ProjectWalkingSim/Private/AI/Tasks/STT_ChasePlayer.cpp` - MoveToActor pursuit, LOS monitoring, grab range check, speed restore
- `Source/ProjectWalkingSim/Public/AI/Tasks/STT_SearchArea.h` - Search task with NavMesh point generation, linger timer, duration timeout
- `Source/ProjectWalkingSim/Private/AI/Tasks/STT_SearchArea.cpp` - Last-known + random search points, periodic look-around, clean state wipe
- `Source/ProjectWalkingSim/Public/AI/Tasks/STT_ReturnToNearestWaypoint.h` - Nearest waypoint task with acceptance radius
- `Source/ProjectWalkingSim/Private/AI/Tasks/STT_ReturnToNearestWaypoint.cpp` - DistSquared nearest waypoint, CurrentWaypointIndex assignment, patrol speed

## Decisions Made
- **MoveToActor for chase (not MoveToLocation):** MoveToActor auto-updates the destination as the player moves, avoiding the need to re-issue movement requests every frame. Only re-issues when path following goes idle mid-chase.
- **Grab range requires LOS:** No blind grabs through walls. The chase task only returns Succeeded (triggering GrabAttack) when both within GrabRange AND LineOfSightTo returns true.
- **SearchRadius 600cm max:** GetRandomReachablePointInRadius degrades with large radii (>1500cm). 600cm is reliable and creates a tight, intentional search pattern.
- **SearchArea ExitState wipes all chase/search state:** After search completes, LastKnownPlayerLocation and WitnessedHidingSpot are cleared. This prevents stale data from influencing the next alert cycle.

## Deviations from Plan

### Note: Task 1 Pre-implemented

Task 1 (STT_ChasePlayer) was already committed in a prior session as part of commit `9fb454f`. The code matched the plan specification exactly. Task 2 (SearchArea + ReturnToNearestWaypoint) was the remaining work executed in this session.

## Issues Encountered
None.

## User Setup Required
None - no external service configuration required.

## Next Phase Readiness
- Complete hunt cycle: Chase -> Search -> ReturnToWaypoint -> Patrol
- All three tasks follow established FSTT_ pattern for State Tree wiring
- All tasks restore WendigoWalkSpeed in ExitState (no speed leak risk)
- Ready for State Tree asset wiring (connecting chase/search/return transitions)

---
*Phase: 05-monster-behaviors*
*Completed: 2026-02-12*
