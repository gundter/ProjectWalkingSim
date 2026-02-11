---
phase: 04-monster-ai-core
plan: 02
subsystem: ai
tags: [ai, suspicion, wendigo, perception, character, ue5-statetree]

# Dependency graph
requires:
  - phase: 04-monster-ai-core/04-01
    provides: "EAlertLevel enum, FOnAlertLevelChanged delegate, AIConstants namespace"
provides:
  - "USuspicionComponent: suspicion accumulation/decay, alert level management, delegate broadcasts"
  - "AWendigoCharacter: tall AI pawn with SuspicionComponent, PatrolRoute ref, slow walk speed"
  - "APatrolRouteActor stub: waypoint array with wrapping GetWaypoint()"
affects: [04-03-ai-controller, 04-04-patrol-route, 05-monster-behaviors]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Suspicion accumulation scaled by normalized visibility above threshold"
    - "Three-level alert system with delegate broadcasts on transitions"
    - "Forward-declare + stub pattern for unrealized dependencies (PatrolRouteActor)"

key-files:
  created:
    - "Source/ProjectWalkingSim/Public/AI/SuspicionComponent.h"
    - "Source/ProjectWalkingSim/Private/AI/SuspicionComponent.cpp"
    - "Source/ProjectWalkingSim/Public/AI/WendigoCharacter.h"
    - "Source/ProjectWalkingSim/Private/AI/WendigoCharacter.cpp"
    - "Source/ProjectWalkingSim/Public/AI/PatrolRouteActor.h"
    - "Source/ProjectWalkingSim/Private/AI/PatrolRouteActor.cpp"
  modified: []

key-decisions:
  - "Visibility normalization: (score - threshold) / (1 - threshold) maps above-threshold to 0-1"
  - "PatrolRouteActor stub created early to satisfy UHT UPROPERTY resolution"
  - "Hearing adds fixed suspicion bump (0.25) plus records stimulus location"

patterns-established:
  - "Suspicion component: external callers drive ProcessSight/Hearing/Decay, component manages state"
  - "Wrapping waypoint index: double-modulo handles negative values safely"

# Metrics
duration: 7min
completed: 2026-02-11
---

# Phase 4 Plan 02: Wendigo Character and Suspicion System Summary

**Wendigo pawn (260cm capsule, 150 cm/s walk) with SuspicionComponent implementing visibility-scaled accumulation, hearing bumps, 15s decay, and three-level alert broadcasts**

## Performance

- **Duration:** 7 min
- **Started:** 2026-02-11T15:51:14Z
- **Completed:** 2026-02-11T15:57:45Z
- **Tasks:** 2
- **Files created:** 6

## Accomplishments
- USuspicionComponent with ProcessSightStimulus (visibility-scaled), ProcessHearingStimulus (fixed bump + location), and DecaySuspicion (~15s full decay)
- Three alert levels (Patrol at 0.0, Suspicious at 0.4, Alert at 0.8) with FOnAlertLevelChanged delegate broadcasts
- AWendigoCharacter with 260cm capsule, 150 cm/s walk, auto-possession by AWendigoAIController
- APatrolRouteActor stub with waypoint array and wrapping GetWaypoint() utility

## Task Commits

Each task was committed atomically:

1. **Task 1: SuspicionComponent** - `328f5c4` (feat) - committed by prior session alongside AITypes rename and WendigoAIController
2. **Task 2: WendigoCharacter** - `28c34d7` (feat) - WendigoCharacter pawn + PatrolRouteActor stub

**Plan metadata:** (pending)

## Files Created/Modified
- `Source/ProjectWalkingSim/Public/AI/SuspicionComponent.h` - Suspicion accumulation/decay component with 3 alert levels
- `Source/ProjectWalkingSim/Private/AI/SuspicionComponent.cpp` - Sight/hearing processing, decay, alert level transitions with logging
- `Source/ProjectWalkingSim/Public/AI/WendigoCharacter.h` - 260cm tall ACharacter with SuspicionComponent and PatrolRoute
- `Source/ProjectWalkingSim/Private/AI/WendigoCharacter.cpp` - Constructor setting capsule, movement, AI controller, subobjects
- `Source/ProjectWalkingSim/Public/AI/PatrolRouteActor.h` - Stub waypoint container actor for patrol paths
- `Source/ProjectWalkingSim/Private/AI/PatrolRouteActor.cpp` - GetWaypoint with wrapping index

## Decisions Made
- **Visibility normalization above threshold:** (score - threshold) / (1.0 - threshold) gives clean 0-1 range that maps threshold edge to 0.0 and full visibility to 1.0. This means a player at exactly 0.3 visibility generates zero suspicion, while 0.65 generates half-rate suspicion.
- **Fixed hearing bump (0.25) rather than scaled:** A single sprint footstep jumps suspicion to 0.25, which is below the 0.4 Suspicious threshold. Two footsteps push to 0.50 (Suspicious). This matches the horror game convention where a single noise gets attention but doesn't trigger pursuit.
- **PatrolRouteActor stub created early:** UHT requires UPROPERTY class types to exist at parse time. Created a minimal PatrolRouteActor with the core waypoint array so WendigoCharacter could compile. Will be expanded with editor visualization in a later plan.
- **HearingSuspicionBump not in AIConstants:** Unlike other tuning params that have centralized defaults, the hearing bump is a SuspicionComponent-specific value (0.25f) set as an editable UPROPERTY. This is because it's a higher-level gameplay parameter rather than a raw perception constant.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Created PatrolRouteActor stub to satisfy UHT**
- **Found during:** Task 2 (WendigoCharacter)
- **Issue:** UHT could not resolve forward-declared APatrolRouteActor class for TObjectPtr UPROPERTY. Compilation failed with "Unable to find 'class' with name 'APatrolRouteActor'"
- **Fix:** Created minimal APatrolRouteActor with TArray<FVector> Waypoints and GetWaypoint() utility
- **Files created:** PatrolRouteActor.h, PatrolRouteActor.cpp
- **Verification:** Project compiles successfully (7/7 build actions)
- **Committed in:** 28c34d7 (Task 2 commit)

**2. [Note] Task 1 committed by prior session**
- **Context:** SuspicionComponent files were committed in `328f5c4` by a prior session/linter alongside the AITypes.h -> MonsterAITypes.h rename and WendigoAIController creation
- **Impact:** Task 1 artifacts were already present and verified; no re-work needed
- **Verification:** UBT confirmed "Target is up to date" before Task 2 changes

---

**Total deviations:** 1 auto-fixed (1 blocking)
**Impact on plan:** PatrolRouteActor stub is forward-compatible with the full implementation planned later. No scope creep.

## Issues Encountered
- AITypes.h was renamed to MonsterAITypes.h by a prior session to avoid potential engine header conflicts. SuspicionComponent.h already had the updated include. No action needed.
- WendigoAIController was pre-created by a prior session (commit 328f5c4). Task 2's WendigoCharacter references it via AIControllerClass with no issues.

## User Setup Required
None - no external service configuration required.

## Next Phase Readiness
- Wendigo pawn and suspicion system are ready for AI controller integration (Plan 04-03)
- PatrolRouteActor stub ready for expansion with editor visualization (Plan 04-04)
- SuspicionComponent's ProcessSightStimulus/ProcessHearingStimulus API ready to be called from WendigoAIController's perception delegate

---
*Phase: 04-monster-ai-core*
*Completed: 2026-02-11*
