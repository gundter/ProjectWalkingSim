---
phase: 05-monster-behaviors
plan: 03
subsystem: ai
tags: [state-tree, grab-attack, investigation, stimulus-type, condition]

# Dependency graph
requires:
  - phase: 04-monster-ai-core
    provides: "State Tree tasks (InvestigateLocation, PatrolMoveToWaypoint), conditions (SuspicionLevel), SuspicionComponent, MonsterAITypes"
  - phase: 05-monster-behaviors-01
    provides: "EWendigoBehaviorState, EStimulusType, LastStimulusType tracking, WendigoInvestigateSightSpeed constant"
provides:
  - "FSTT_GrabAttack: cinematic kill sequence State Tree task (disable input, wait, restart level)"
  - "FSTC_StimulusType: State Tree condition for stimulus type branching (sight vs sound)"
  - "Enhanced STT_InvestigateLocation with stimulus-type-aware speed differentiation"
affects: [05-monster-behaviors-04, 05-monster-behaviors-05, 08-demo-polish]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Stimulus-type-aware speed selection: bUseStimulusTypeSpeed flag with EStimulusType enum check"
    - "Input disable safety pattern: track bInputDisabled in instance data, restore in ExitState"

key-files:
  created:
    - "Source/ProjectWalkingSim/Public/AI/Tasks/STT_GrabAttack.h"
    - "Source/ProjectWalkingSim/Private/AI/Tasks/STT_GrabAttack.cpp"
    - "Source/ProjectWalkingSim/Public/AI/Conditions/STC_StimulusType.h"
    - "Source/ProjectWalkingSim/Private/AI/Conditions/STC_StimulusType.cpp"
  modified:
    - "Source/ProjectWalkingSim/Public/AI/Tasks/STT_InvestigateLocation.h"
    - "Source/ProjectWalkingSim/Private/AI/Tasks/STT_InvestigateLocation.cpp"

key-decisions:
  - "RestartLevel console command for demo-scope death; Phase 8 replaces with proper death/respawn system"
  - "bUseStimulusTypeSpeed defaults true for new behavior while preserving backward compatibility"

patterns-established:
  - "Input disable safety: always track disabled state in instance data and restore in ExitState to prevent permanent input lock on interrupted State Tree transitions"
  - "BehaviorState lifecycle: set in EnterState, restore to Patrol in ExitState for each task"

# Metrics
duration: 6min
completed: 2026-02-12
---

# Phase 5 Plan 3: Grab Attack, Stimulus Condition, and Investigation Enhancement Summary

**GrabAttack kill sequence with input disable/restore safety, StimulusType condition for State Tree branching, and investigation speed differentiation (250 cm/s sight, 200 cm/s sound)**

## Performance

- **Duration:** ~6 min
- **Started:** 2026-02-12T14:58:24Z
- **Completed:** 2026-02-12T15:04:09Z
- **Tasks:** 2
- **Files modified:** 6 (4 created, 2 modified)

## Accomplishments
- FSTT_GrabAttack: cinematic kill sequence that disables player input, waits GrabDuration (2s), then restarts level; safely re-enables input in ExitState for interrupted transitions
- FSTC_StimulusType: State Tree condition reading SuspicionComponent::GetLastStimulusType() with RequiredType comparison and optional invert; follows established FSTC_ pattern with empty instance data
- STT_InvestigateLocation enhanced with stimulus-type-aware speed: sight stimuli trigger 250 cm/s (aggressive), sound stimuli use 200 cm/s (cautious); BehaviorState tracking added

## Task Commits

Each task was committed atomically:

1. **Task 1: Create STT_GrabAttack and STC_StimulusType** - `67c3ae9` (feat)
2. **Task 2: Enhance STT_InvestigateLocation** - `c1f9d04` (feat)

## Files Created/Modified
- `Source/ProjectWalkingSim/Public/AI/Tasks/STT_GrabAttack.h` - Cinematic kill task with ControllerHandle, GrabDuration, CameraBlendTime
- `Source/ProjectWalkingSim/Private/AI/Tasks/STT_GrabAttack.cpp` - EnterState disables input + faces player; Tick waits then RestartLevel; ExitState safety restore
- `Source/ProjectWalkingSim/Public/AI/Conditions/STC_StimulusType.h` - Condition with RequiredType (EStimulusType) and bInvertCondition
- `Source/ProjectWalkingSim/Private/AI/Conditions/STC_StimulusType.cpp` - TestCondition reads LastStimulusType from SuspicionComponent
- `Source/ProjectWalkingSim/Public/AI/Tasks/STT_InvestigateLocation.h` - Added InvestigationSightSpeed (250) and bUseStimulusTypeSpeed properties
- `Source/ProjectWalkingSim/Private/AI/Tasks/STT_InvestigateLocation.cpp` - Speed selection branch and BehaviorState lifecycle

## Decisions Made
- **RestartLevel for demo death**: Simple console command approach for demo scope. Phase 8 Polish will replace with proper death screen / checkpoint reload. The GrabAttack task structure is designed to accommodate this -- only the death handler in Tick changes.
- **bUseStimulusTypeSpeed defaults true**: New behavior is the desired default, but the flag allows backward compatibility if any existing State Tree references need the old fixed-speed behavior.
- **BehaviorState lifecycle in tasks**: Each State Tree task now sets BehaviorState in EnterState and restores to Patrol in ExitState, ensuring the next task's EnterState starts from a clean state.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Fixed STT_ChasePlayer.h missing MonsterAITypes.h include**
- **Found during:** Task 1 (compilation verification)
- **Issue:** Pre-existing 05-02 file STT_ChasePlayer.h used AIConstants:: without including MonsterAITypes.h
- **Fix:** Added `#include "AI/MonsterAITypes.h"` to the header
- **Files modified:** Source/ProjectWalkingSim/Public/AI/Tasks/STT_ChasePlayer.h
- **Verification:** Build succeeded after fix
- **Committed in:** 9fb454f (pre-existing 05-02 commit)

**2. [Rule 3 - Blocking] Fixed WendigoSpawnPoint.cpp SpawnActor signature for UE5.7**
- **Found during:** Task 1 (compilation verification)
- **Issue:** Pre-existing 05-02 file used `SpawnActor<T>(UClass*, const FTransform*, FActorSpawnParameters)` which doesn't exist in UE5.7; correct 4-arg overload takes `(UClass*, const FVector&, const FRotator&, FActorSpawnParameters)`
- **Fix:** Changed to use location/rotation references instead of transform pointer
- **Files modified:** Source/ProjectWalkingSim/Private/AI/WendigoSpawnPoint.cpp
- **Verification:** Build succeeded after fix
- **Committed in:** 9fb454f (pre-existing 05-02 commit)

---

**Total deviations:** 2 auto-fixed (2 blocking)
**Impact on plan:** Both fixes were in pre-existing 05-02 files that had never been committed. Required to achieve clean compilation. No scope creep.

## Issues Encountered
None during planned work. The two pre-existing compilation errors from 05-02 were the only obstacles.

## User Setup Required
None - no external service configuration required.

## Next Phase Readiness
- GrabAttack task ready for State Tree wiring (chase -> grab attack transition on GrabRange)
- StimulusType condition ready for branching investigation states in State Tree
- Investigation speed differentiation active by default
- Remaining Phase 5 plans: search behavior (05-04) and spawn system (05-05)

---
*Phase: 05-monster-behaviors*
*Completed: 2026-02-12*
