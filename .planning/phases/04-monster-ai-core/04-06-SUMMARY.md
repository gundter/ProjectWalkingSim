---
phase: 04-monster-ai-core
plan: 06
subsystem: ai
tags: [state-tree, investigation, orient, condition, alert-level, suspicion, menacing-pause]

# Dependency graph
requires:
  - phase: 04-04
    provides: "FSTT_PatrolMoveToWaypoint, FSTT_PatrolIdle task patterns and conventions"
  - phase: 04-05
    provides: "Perception-to-suspicion pipeline, SuspicionComponent with alert levels and stimulus location"
provides:
  - "STT_InvestigateLocation: navigates to stimulus at 200 cm/s with post-arrival look-around"
  - "STT_OrientToward: menacing pause -- stop and turn toward stimulus for 2 seconds"
  - "STC_SuspicionLevel: alert level threshold condition for State Tree transitions"
  - "Complete Wendigo reactive behavior set for Phase 4 (patrol + investigate + orient + condition)"
affects: [04-07, 05-monster-behaviors]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "FStateTreeConditionCommonBase for State Tree conditions (lightweight enum comparison)"
    - "Speed override in EnterState, restore in ExitState (prevent speed leaks across state transitions)"
    - "SetFocalPoint for smooth AI rotation via UpdateControlRotation (no manual rotation math)"

key-files:
  created:
    - "Source/ProjectWalkingSim/Public/AI/Tasks/STT_InvestigateLocation.h"
    - "Source/ProjectWalkingSim/Private/AI/Tasks/STT_InvestigateLocation.cpp"
    - "Source/ProjectWalkingSim/Public/AI/Tasks/STT_OrientToward.h"
    - "Source/ProjectWalkingSim/Private/AI/Tasks/STT_OrientToward.cpp"
    - "Source/ProjectWalkingSim/Public/AI/Conditions/STC_SuspicionLevel.h"
    - "Source/ProjectWalkingSim/Private/AI/Conditions/STC_SuspicionLevel.cpp"
  modified: []

key-decisions:
  - "FStateTreeConditionCommonBase as condition base class (not FStateTreeConditionBase) -- CommonBase is the schema-safe subclass"
  - "No instance data for condition -- pure stateless enum comparison"
  - "InvestigateLocation clears stimulus on completion so patrol can resume naturally"

patterns-established:
  - "Speed override + restore pattern for temporary movement changes across State Tree states"
  - "STC_ prefix for State Tree condition structs (parallel to STT_ for tasks)"

# Metrics
duration: 5min
completed: 2026-02-11
---

# Phase 4 Plan 06: Investigation Tasks and Suspicion Condition Summary

**InvestigateLocation navigates to stimulus at 200 cm/s with 4s look-around; OrientToward creates menacing 2s pause; SuspicionLevel condition gates State Tree alert transitions**

## Performance

- **Duration:** ~5 min
- **Started:** 2026-02-11T16:17:31Z
- **Completed:** 2026-02-11T16:22:30Z
- **Tasks:** 2/2
- **Files created:** 6

## Accomplishments
- STT_InvestigateLocation: reads stimulus from SuspicionComponent, moves at 200 cm/s (faster than patrol 150, slower than player 250), 4s look-around phase, clears stimulus on completion, restores walk speed in ExitState
- STT_OrientToward: stops movement, sets focal point for smooth rotation via UpdateControlRotation, holds for 2s menacing pause, supports SuspicionComponent read or property binding
- STC_SuspicionLevel: lightweight condition comparing EAlertLevel >= RequiredLevel with bInvertCondition for return-to-patrol transitions
- All three structs appear in State Tree editor with correct display names

## Task Commits

Each task was committed atomically:

1. **Task 1: InvestigateLocation and OrientToward tasks** - `a86cd8a` (feat)
   - FSTT_InvestigateLocation: two-phase execution (navigate + look-around), speed override/restore
   - FSTT_OrientToward: menacing pause with SetFocalPoint, configurable duration
   - Both follow established STT_ pattern from 04-04 tasks
2. **Task 2: SuspicionLevel condition** - `c54ed94` (feat)
   - FSTC_SuspicionLevel: FStateTreeConditionCommonBase subclass
   - Enum comparison with invert option for bidirectional transitions
   - New STC_ prefix convention for conditions directory

**Plan metadata:** (pending)

## Files Created/Modified
- `Source/ProjectWalkingSim/Public/AI/Tasks/STT_InvestigateLocation.h` - Investigation task: navigate to stimulus, look around, clear
- `Source/ProjectWalkingSim/Private/AI/Tasks/STT_InvestigateLocation.cpp` - Two-phase state machine: MoveToLocation then timed look-around
- `Source/ProjectWalkingSim/Public/AI/Tasks/STT_OrientToward.h` - Orient task: stop and face stimulus for configurable duration
- `Source/ProjectWalkingSim/Private/AI/Tasks/STT_OrientToward.cpp` - SetFocalPoint for smooth rotation, timer-based completion
- `Source/ProjectWalkingSim/Public/AI/Conditions/STC_SuspicionLevel.h` - Condition: EAlertLevel threshold check with invert
- `Source/ProjectWalkingSim/Private/AI/Conditions/STC_SuspicionLevel.cpp` - Stateless enum comparison, no instance data needed

## Decisions Made
- **FStateTreeConditionCommonBase (not FStateTreeConditionBase):** CommonBase is the schema-safe subclass that allows State Tree schemas to safely include all common conditions. Verified from engine source at `StateTreeConditionBase.h`.
- **No instance data for condition:** The suspicion level check is a pure stateless evaluation -- reads alert level, compares to threshold, returns bool. No need for per-instance tracking.
- **InvestigateLocation clears stimulus on completion:** After the Wendigo finishes investigating and looking around, ClearStimulusLocation is called so the State Tree can naturally transition back to patrol without stale stimulus data.
- **STC_ prefix for condition structs:** Establishes naming convention parallel to STT_ for tasks (STC = State Tree Condition).

## Deviations from Plan

None -- plan executed exactly as written.

## Issues Encountered
None.

## User Setup Required
None -- no external service configuration required.

## Next Phase Readiness
- All Phase 4 C++ tasks and conditions are now complete: patrol (move + idle), investigate, orient, suspicion check
- Ready for 04-07: State Tree asset creation in editor, NavMesh configuration, PIE verification
- The condition enables visual authoring of state transitions in the State Tree editor
- Phase 5 (Monster Behaviors) can extend these tasks for chase, search, and spawn behaviors

---
*Phase: 04-monster-ai-core*
*Completed: 2026-02-11*
